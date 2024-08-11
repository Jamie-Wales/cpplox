#include "vMachine.h"
#include "Instructions.h"
#include <iostream>
Value vMachine::readConstant()
{
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
            case RETURN: {
                if (stack.size() > 0)
                    stack.pop();
                state = vState::OK;
                return;
            }
            case CONSTANT: {
                Value constant = readConstant();
                stack.push(constant);
                break;
            }
            case CONSTANT_LONG: {
                Value constant = readConstantLong();
                stack.push(constant);
                break;
            }
            case ADD:
                ensureStackSize(2, "ADD");
                add();
                break;
            case MULT:
                ensureStackSize(2, "MULTIPLY");
                mult();
                break;
            case DIV:
                ensureStackSize(2, "DIVIDE");
                div();
                break;
            case NEG:
                ensureStackSize(1, "NEGATE");
                neg();
                break;
            case PRINT:
                ensureStackSize(1, "PRINT");
                stack.top().print();
                stack.pop();
                std::cout << std::endl;
                break;
            case POP:
                stack.pop();
                break;
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
