#include "vMachine.h"
#include "Instructions.h"
#include "Object.h"
#include "Stringinterner.h"
#include "Visit.h"
#include <cstdint>
#include <ctime>
#include <format>
#include <iostream>
#include <ostream>
#include <string>
#include <variant>
#define DEBUG_TRACE_EXECUTION
size_t& vMachine::ip()
{
    return frames.back().ip;
}

Chunk& vMachine::instructions() const
{
    if (frames.back().function != nullptr) {
        return frames.back().function->chunk;
    } else {
        return frames.back().closure->pFunction->chunk;
    }
}

void vMachine::runtimeError(const std::string& error)
{
    std::cerr << error << std::endl;
    const size_t instruction = ip() - 1;
    const int line = instructions().lines[instruction].lineNumber;
    std::cerr << "[line " << line << "] in script\n";
    resetStack();
}

void vMachine::defineNative(const std::string& name, NativeFn function)
{
    auto internedName = StringInterner::instance().intern(name);
    auto nativeObj = new Obj(ObjNative(std::move(function)));
    globals[*internedName] = Value(nativeObj);
}

int16_t vMachine::readShort()
{
    uint8_t high = instructions().code[ip()];
    uint8_t low = instructions().code[ip() + 1];
    ip() += 2;
    int16_t result = (high << 8) | low;
    return result;
}

Value vMachine::readConstant()
{
    if (ip() > 255)
        return readConstantLong();
    return instructions().pool[instructions().code[ip()++]];
}

Value vMachine::readConstantLong()
{
    const uint32_t index = instructions().code[ip()] | (instructions().code[ip() + 1] << 8) | (instructions().code[ip() + 2] << 16);
    ip() += 3;
    return instructions().pool[index];
}

class StackUnderflowError final : public std::runtime_error {
public:
    explicit StackUnderflowError(const std::string& opcode)
        : std::runtime_error(std::format("Stack underflow occurred during {} operation", opcode))
    {
    }
};

void vMachine::ensureStackSize(size_t size, const char* opcode) const
{
    if (stack.size() < size) {
        throw StackUnderflowError(opcode);
    }
}

uint8_t vMachine::readByte()
{
    return instructions().code[ip()++];
}
void vMachine::closeUpvalues(Value* last)
{
    while (openUpvalues != nullptr && openUpvalues->location >= last) {
        ObjUpvalue* upvalue = openUpvalues;
        upvalue->closed = *upvalue->location;
        upvalue->location = &upvalue->closed;
        openUpvalues = upvalue->next;
    }
}

void vMachine::execute()
{
    auto main = frames.back();
    frames.clear();
    frames.push_back(main);
    ip() = 0;
    run();
}

CallFrame& vMachine::frame()
{
    return frames.back();
}

size_t vMachine::offset()
{
    return frame().stackOffset;
}

ObjUpvalue* vMachine::captureUpvalue(Value* local)
{

    ObjUpvalue* prevUpvalue = nullptr;
    ObjUpvalue* upvalue = this->openUpvalues;
    while (upvalue != nullptr && upvalue->location > local) {
        prevUpvalue = upvalue;
        upvalue = upvalue->next;
    }

    if (upvalue != nullptr && upvalue->location == local) {
        return upvalue;
    }
    auto* createdUpvalue = new ObjUpvalue { local };

    createdUpvalue->next = upvalue;

    if (prevUpvalue == nullptr) {
        openUpvalues = createdUpvalue;
    } else {
        prevUpvalue->next = createdUpvalue;
    }
    return createdUpvalue;
}

void vMachine::run()
{
    try {
        while (ip() < instructions().code.size()) {
            uint8_t byte = readByte();
#ifdef DEBUG_TRACE_EXECUTION
            std::cout << "          ";
            for (const auto& value : stack) {
                std::cout << "[ ";
                value.print();
                std::cout << " ]";
            }
            std::cout << "\n";
            instructions().disassembleInstruction(ip() - 1);
#endif
            switch (byte) {
            case cast(OP_CODE::CALL): {
                int argCount = readByte();
                if (!callValue(stack[stack.size() - 1 - argCount - offset()], argCount)) {
                    return;
                }
                break;
            }
            case cast(OP_CODE::CLOSURE): {
                Value funcAsValue = readConstant();
                auto function = funcAsValue.asFunc();
                auto closure = new ObjClosure { function };
                for (int i = 0; i < function->upValueCount; i++) {
                    uint8_t isLocal = readByte();
                    uint8_t index = readByte();
                    if (isLocal) {
                        closure->upValues.emplace_back(captureUpvalue(&stack[offset() + index]));
                    } else {
                        closure->upValues.emplace_back(frames.back().closure->upValues[index]);
                    }
                }
                stack.emplace_back(new Obj { *closure });
                break;
            }
            case cast(OP_CODE::GET_UPVALUE): {
                uint8_t slot = readByte();
                stack.emplace_back(frames.back().closure->upValues[slot]->location);
                break;
            }
            case cast(OP_CODE::SET_UPVALUE): {
                uint8_t slot = readByte();
                frames.back().closure->upValues[slot]->location = &stack.back();
                break;
            }
            case cast(OP_CODE::NIL): {
                stack.emplace_back(Value { nullptr });
                break;
            }
            case cast(OP_CODE::RETURN): {
                Value result = stack.back();
                stack.resize(frames.back().stackOffset - 1);
                closeUpvalues(&stack[frames.back().stackOffset - 1]);
                frames.pop_back();
                if (frames.size() == 1) {
                    stack.push_back(result);
                    return;
                }
                stack.push_back(result);
                break;
            }

            case cast(OP_CODE::LOOP): {
                uint16_t offset = readShort();
                ip() -= offset;
            } break;
            case cast(OP_CODE::CONSTANT): {
                Value constant = readConstant();
                stack.push_back(constant);
            } break;
            case cast(OP_CODE::CONSTANT_LONG): {
                Value constant = readConstantLong();
                stack.push_back(constant);
            } break;
            case cast(OP_CODE::TRUE):
                stack.emplace_back(true);
                break;
            case cast(OP_CODE::FALSE):
                stack.emplace_back(false);
                break;
            case cast(OP_CODE::ADD):
                ensureStackSize(2, "ADD");
                add();
                break;
            case cast(OP_CODE::MULT):
                ensureStackSize(2, "MULTIPLY");
                mult();
                break;
            case cast(OP_CODE::DIV):
                ensureStackSize(2, "DIVIDE");
                div();
                break;
            case cast(OP_CODE::NEG):
                ensureStackSize(1, "NEGATE");
                neg();
                break;
            case cast(OP_CODE::PRINT):
                ensureStackSize(1, "PRINT");
                stack.back().print();
                std::cout << std::endl;
                break;
            case cast(OP_CODE::POP):
                stack.pop_back();
                break;
            case cast(OP_CODE::DEFINE_GLOBAL): {
                auto name = readConstant();
                auto value = stack.back();
                if (auto internedString = StringInterner::instance().find(name.to_string()); !internedString) {
                    std::cerr << "Failed to intern string: " << name.to_string() << std::endl;
                } else {
                    if (globals.contains(*internedString)) {
                        runtimeError(std::format("Cannot redefine previously defined variable {}", *internedString));
                    }
                    globals[*internedString] = value;
                    stack.pop_back();
                }
            } break;
            case cast(OP_CODE::SET_GLOBAL): {
                auto name = readConstant();
                globals[name.to_string()] = stack.back();
                break;
            }
            case cast(OP_CODE::GET_GLOBAL): {
                auto name = readConstant();
                auto it = globals.find(name.to_string());
                if (it == globals.end()) {
                    runtimeError(std::format("wUndefined variable {}.", name.to_string()));
                }
                stack.push_back(it->second);
            } break;
            case cast(OP_CODE::GET_LOCAL): {
                uint8_t slot = readByte();
                size_t index = offset() + slot;
#ifdef DEBUG_TRACE_EXECUTION
                std::cout << "Getting local variable at slot " << static_cast<int>(slot)
                          << " (stack index " << index << ")" << std::endl;
#endif
                if (index >= stack.size()) {
                    runtimeError(std::format("Attempt to access invalid local variable at slot {}", slot));
                    return;
                }
                stack.push_back(stack[index]);
            } break;
            case cast(OP_CODE::SET_LOCAL): {
                uint8_t slot = readByte();
                stack[offset() + slot] = stack.back();
            } break;
            case cast(OP_CODE::NOT):
                ensureStackSize(1, "LOGICAL_NOT");
                logicalNot();
                break;
            case cast(OP_CODE::GREATER):
                ensureStackSize(2, "GREATER");
                greater();
                break;
            case cast(OP_CODE::GREATER_EQUAL):
                ensureStackSize(2, "GREATER_EQUAL");
                greaterEqual();
                break;
            case cast(OP_CODE::LESS):
                ensureStackSize(2, "LESS");
                less();
                break;
            case cast(OP_CODE::LESS_EQUAL):
                ensureStackSize(2, "LESS");
                lessEqual();
                break;
            case cast(OP_CODE::EQUAL):
                ensureStackSize(2, "EQUAL");
                equal();
                break;
            case cast(OP_CODE::CLOSE_UPVALUE):
                closeUpvalues(&stack.back());
                stack.pop_back();
                break;
            case cast(OP_CODE::JUMP_IF_FALSE): {
                ensureStackSize(1, "JUMP_IF_FALSE");
                int offset = readShort();
                if (!stack.back().isTruthy()) {
                    ip() += offset;
                }
                break;
            }
            case cast(OP_CODE::JUMP): {
                int offset = readShort();
                ip() += offset;
                break;
            }
            case cast(OP_CODE::SWAP): {
                swap();
                break;
            }
            case cast(OP_CODE::DUP): {
                dup();
                break;
            }
            default:
                throw std::runtime_error(std::format("Unknown opcode: {}", static_cast<int>(byte)));
            }
        }
    } catch (const StackUnderflowError& e) {
        std::cerr << std::format("Runtime Error: {}\n", e.what());
        state = vState::BAD;
        stack.clear();
    } catch (const std::exception& e) {
        std::cerr << std::format("Runtime Error: {}\n", e.what());
        state = vState::BAD;
    }
}

void vMachine::add()
{
    const auto b = stack.back();
    stack.pop_back();
    stack.back() += b;
}

void vMachine::swap()
{
    const auto top = stack.back();
    stack.pop_back();
    const auto next = stack.back();
    stack.pop_back();
    stack.push_back(top);
    stack.push_back(next);
}

void vMachine::mult()
{
    const auto& right = stack.back();
    stack.pop_back();
    stack.back() *= right;
}

void vMachine::div()
{
    const auto& right = stack.back();
    stack.pop_back();
    stack.back() /= right;
}

void vMachine::dup()
{
    stack.push_back(stack.back());
}

void vMachine::neg()
{
    stack.back() = -stack.back();
}

void vMachine::resetStack()
{
    stack.clear();
}

void vMachine::logicalNot()
{
    stack.back() = !stack.back();
}

void vMachine::greater()
{
    const auto b = stack.back();
    stack.pop_back();
    const auto& a = stack.back();
    stack.back() = a > b;
}

void vMachine::less()
{
    const auto b = stack.back();
    stack.pop_back();
    const auto& a = stack.back();
    stack.back() = a < b;
}

void vMachine::equal()
{
    const auto b = stack.back();
    stack.pop_back();
    const auto& a = stack.back();
    stack.back() = a == b;
}

void vMachine::greaterEqual()
{
    const auto b = stack.back();
    stack.pop_back();
    const auto& a = stack.back();
    stack.back() = !(a < b);
}

void vMachine::lessEqual()
{
    const auto b = stack.back();
    stack.pop_back();
    const auto& a = stack.back();
    stack.back() = !(a > b);
}

void vMachine::call(ObjFunction* function, int argCount)
{
    if (argCount != function->arity) {
        runtimeError(std::format("Function expected {} arguments but got {}.", function->arity, argCount));
        return;
    }
    if (frames.size() == FRAMES_MAX) {
        runtimeError("Stack overflow.");
        return;
    }

    CallFrame callFrame {
        function,
        0,
        stack.size() - argCount - 1,
        nullptr
    };
    frames.push_back(callFrame);
}

void vMachine::call(ObjClosure* closure, int argCount)
{
    if (argCount != closure->pFunction->arity) {
        runtimeError(std::format("Closure expected {} arguments but got {}.", closure->pFunction->arity, argCount));
        return;
    }
    if (frames.size() == FRAMES_MAX) {
        runtimeError("Stack overflow.");
        return;
    }

    CallFrame callFrame {
        nullptr,
        0,
        stack.size() - argCount - 1,
        closure
    };
    frames.push_back(callFrame);
}

void vMachine::load(ObjFunction* mainFunction)
{

    frames.emplace_back(CallFrame { nullptr, 0, 0, new ObjClosure { mainFunction } });
    defineNativeFunctions();
}

bool vMachine::callValue(Value callee, int argCount)
{
    return std::visit(overloaded {
                          [this, argCount](Obj* obj) -> bool {
                              return std::visit(overloaded {
                                                    [this, argCount](ObjFunction& func) -> bool {
                                                        call(&func, argCount);
                                                        return true;
                                                    },

                                                    [this, argCount](ObjClosure& cloj) -> bool {
                                                        call(&cloj, argCount);
                                                        return true;
                                                    },
                                                    [this, argCount](ObjNative& native) -> bool {
                                                        const Value result = native.function(argCount, &stack[stack.size() - argCount]);
                                                        stack.pop_back();
                                                        stack.pop_back();
                                                        stack.push_back(result);
                                                        return true;
                                                    },
                                                    [this](const auto&) -> bool {
                                                        runtimeError("Can only call functions and classes.");
                                                        return false;
                                                    } },
                                  obj->as);
                          },
                          [this](const auto& b) -> bool {
                              runtimeError("Cannot call a non Object");
                              return false;
                          } },
        callee.as);
}
