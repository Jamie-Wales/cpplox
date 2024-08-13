#include "vMachine.h"
#include "Instructions.h"
#include <iostream>
#include <ostream>
#include <sstream>
#include <string>
#define DEBUG_TRACE_EXECUTION

template <typename... Args>
void vMachine::runtimeError(const char* format, Args&&... args)
{
    std::ostringstream error_stream;
    (error_stream << ... << std::forward<Args>(args));
    std::string error_message = error_stream.str();

    std::cerr << error_message << '\n';

    size_t instruction = ip - 1;
    int line = instructions.lines[instruction].lineNumber;
    std::cerr << "[line " << line << "] in script\n";
    resetStack();
}

Value vMachine::readConstant()
{
    if (ip > 255)
        return readConstantLong();
    return instructions.pool[instructions.code[ip++]];
}

Value vMachine::readConstantLong()
{
    uint32_t index = instructions.code[ip] | (instructions.code[ip + 1] << 8) | (instructions.code[ip + 2] << 16);
    ip += 3;
    return instructions.pool[index];
}

class StackUnderflowError : public std::runtime_error {
public:
    StackUnderflowError(const std::string& opcode)
        : std::runtime_error(std::format("Stack underflow occurred during {} operation", opcode))
    {
    }
};

void vMachine::ensureStackSize(size_t size, const char* opcode)
{
    if (stack.size() < size) {
        throw StackUnderflowError(opcode);
    }
}

void vMachine::run()
{
    try {
        while (ip < instructions.code.size()) {
            uint8_t byte = instructions.code[ip++];
#ifdef DEBUG_TRACE_EXECUTION
            std::cout << std::format("          ");
            std::stack<Value> tempStack = stack;
            while (!tempStack.empty()) {
                std::cout << std::format("[ ");
                tempStack.top().print();
                std::cout << std::format(" ]");
                tempStack.pop();
            }
            std::cout << std::format("\n");
            instructions.disassembleInstruction(ip - 1);
#endif
            switch (byte) {
            case cast(OP_CODE::RETURN): {
                if (stack.size() > 0)
                    stack.pop();
                state = vState::OK;
                return;
            }
            case cast(OP_CODE::CONSTANT): {
                Value constant = readConstant();
                stack.push(constant);
            } break;
            case cast(OP_CODE::CONSTANT_LONG): {
                Value constant = readConstantLong();
                stack.push(constant);
                break;
            }
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
                stack.top().print();
                stack.pop();
                std::cout << std::endl;
                break;
            case cast(OP_CODE::POP):
                stack.pop();
                break;
            case cast(OP_CODE::DEFINE_GLOBAL): {
                auto variable = stack.top();
                stack.pop();
                auto name = stack.top();
                stack.pop();
                globals[name.to_string()] = variable;
            } break;
            case cast(OP_CODE::GET_GLOBAL): {
                auto constant = readConstant();
                constant.print();
            } break;
            default:
                throw std::runtime_error(std::format("Unknown opcode: {}", static_cast<int>(byte)));
            }
        }
    } catch (const StackUnderflowError& e) {
        std::cerr << std::format("Runtime Error: {}\n", e.what());
        state = vState::BAD;
        stack = std::stack<Value>(); // Clear the stack
    } catch (const std::exception& e) {
        std::cerr << std::format("Runtime Error: {}\n", e.what());
        state = vState::BAD;
    }
}
void vMachine::add()
{
    auto right = stack.top();
    stack.pop();
    stack.top() += right;
}

void vMachine::mult()
{

    auto right = stack.top();
    stack.pop();
    stack.top() *= right;
}

void vMachine::div()
{
    auto right = stack.top();
    stack.pop();
    stack.top() /= right;
}

void vMachine::neg()
{
    stack.top() = -stack.top();
}
