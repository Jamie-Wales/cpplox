#include "vMachine.h"
#include "Instructions.h"
#include <Stringinterner.h>
#include <cstdint>
#include <format>
#include <iostream>
#include <ostream>
#include <string>
#define DEBUG_TRACE_EXECUTION
template <typename... Args>
void vMachine::runtimeError(const char* format, Args&&... args)
{
    char buffer[1024];
    snprintf(buffer, sizeof(buffer), format, std::forward<Args>(args)...);
    const std::string error_message(buffer);

    std::cerr << error_message << '\n';

    const size_t instruction = ip - 1;
    const int line = instructions.lines[instruction].lineNumber;
    std::cerr << "[line " << line << "] in script\n";
    resetStack();
}

int vMachine::readShort()
{
    ip += 2;
    return ((instructions.code[ip - 2] << 8) | instructions.code[ip - 1]);
}

Value vMachine::readConstant()
{
    if (ip > 255)
        return readConstantLong();
    return instructions.pool[instructions.code[ip++]];
}

Value vMachine::readConstantLong()
{
    const uint32_t index = instructions.code[ip] | (instructions.code[ip + 1] << 8) | (instructions.code[ip + 2] << 16);
    ip += 3;
    return instructions.pool[index];
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

void vMachine::execute(const Chunk& newInstructions)
{
    instructions = newInstructions;
    ip = 0;
    run();
}

void vMachine::run()
{
    try {
        while (ip < instructions.code.size()) {
            uint8_t byte = instructions.code[ip++];
#ifdef DEBUG_TRACE_EXECUTION
            std::cout << "          ";
            for (const auto& value : stack) {
                std::cout << "[ ";
                value.print();
                std::cout << " ]";
            }
            std::cout << "\n";
            instructions.disassembleInstruction(ip - 1);
#endif
            switch (byte) {
            case cast(OP_CODE::LOOP): {
                uint16_t offset = readShort();
                ip -= offset;
            } break;
            case cast(OP_CODE::RETURN): {
                if (!stack.empty())
                    stack.pop_back();
                state = vState::OK;
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
                stack.emplace_back( true );
                break;
            case cast(OP_CODE::FALSE):
                stack.emplace_back( false );
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
                stack.pop_back();
                std::cout << std::endl;
                break;
            case cast(OP_CODE::POP):
                stack.pop_back();
                break;
            case cast(OP_CODE::DEFINE_GLOBAL): {
                auto name = readConstant();
                auto value = stack.back();
                stack.pop_back();
                auto internedString = StringInterner::instance().find(name.to_string());
                if (!internedString) {
                    std::cerr << "Failed to intern string: " << name.to_string() << std::endl;
                } else {
                    if (globals.contains(*internedString)) {
                        runtimeError("Cannot redefine previously defined variable {}", internedString->c_str());
                    }
                    globals[*internedString] = value;
                }
                stack.pop_back();
            } break;
            case cast(OP_CODE::SET_GLOBAL): {
                auto name = readConstant();
                globals[name.to_string()] = stack.back();
            } break;
            case cast(OP_CODE::GET_GLOBAL): {
                auto name = readConstant();
                auto it = globals.find(name.to_string());
                if (it == globals.end()) {
                    runtimeError("Undefined variable {}.", name.to_string().c_str());
                }
                stack.push_back(it->second);
            } break;
            case cast(OP_CODE::GET_LOCAL): {
                uint8_t slot = instructions.code[ip++];
                stack.push_back(stack[slot]);
            } break;
            case cast(OP_CODE::SET_LOCAL): {
                uint8_t slot = instructions.code[ip++];
                stack[slot] = stack.back();
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
            case cast(OP_CODE::JUMP_IF_FALSE): {
                ensureStackSize(1, "JUMP_IF_FALSE");
                int offset = readShort();
                if (!stack.back().isTruthy()) {
                    ip += offset;
                }
            } break;
            case cast(OP_CODE::JUMP): {
                int offset = readShort();
                ip += offset;
            } break;
            case cast(OP_CODE::SWAP): {
                swap();
            } break;
            case cast(OP_CODE::DUP): {
                dup();
            } break;
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
