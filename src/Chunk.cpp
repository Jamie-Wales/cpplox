#include "Chunk.h"
#include "Instructions.h"
#include <format>
#include <iostream>
#include <string>

void Chunk::writeConstant(double value, int line)
{
    int index = addConstant(Value { value });
    if (index < 256) {
        writeChunk(CONSTANT, line);
        writeChunk(index, line);
    } else {
        writeChunk(CONSTANT_LONG, line);
        writeChunk((index & 0xff), line);
        writeChunk(((index >> 8) & 0xff), line);
        writeChunk(((index >> 16) & 0xff), line);
    }
}

void Chunk::writeChunk(uint8_t byte, int line)
{
    code.push_back(byte);
    auto itr = std::find_if(lines.begin(), lines.end(), [line](const LineInfo& element) {
        return element.lineNumber == line;
    });
    if (itr == lines.end()) {
        int size = code.size() - 1;
        lines.push_back({ size, line });
    }
}

int Chunk::disassembleInstruction(int offset)
{
    std::cout << std::format("{:04d} ", offset);
    printLineNumber(offset);
    uint8_t instruction = code[offset];
    switch (instruction) {
    case RETURN:
        return simpleInstruction("OP_RETURN", offset);
    case CONSTANT:
        return constantInstruction("OP_CONSTANT", offset);
    case CONSTANT_LONG:
        return constantLongInstruction("OP_CONSTANT_LONG", offset);
    case ADD:
        return simpleInstruction("OP_ADD", offset);
    case MULT:
        return simpleInstruction("OP_MULTIPLY", offset);
    case DIV:
        return simpleInstruction("OP_DIVIDE", offset);
    case NEG:
        return simpleInstruction("OP_NEGATE", offset);
    case NIL:
        return simpleInstruction("OP_NIL", offset);
    case TRUE:
        return simpleInstruction("OP_TRUE", offset);
    case FALSE:
        return simpleInstruction("OP_FALSE", offset);
    case NOT:
        return simpleInstruction("OP_NOT", offset);
    case GREATER:
        return simpleInstruction("OP_GREATER", offset);
    case LESS:
        return simpleInstruction("OP_LESS", offset);
    case EQUAL:
        return simpleInstruction("OP_EQUAL", offset);
    default:
        std::cout << std::format("Unknown opcode {}\n", instruction);
        return offset + 1;
    }
}

void Chunk::printLineNumber(int offset)
{
    for (const auto& lineInfo : lines) {
        if (lineInfo.offset == offset) {
            std::cout << std::format("{:4d} ", lineInfo.lineNumber);
            return;
        }
    }
    std::cout << "   | ";
}

int Chunk::constantInstruction(const std::string& name, int offset)
{
    uint8_t constant = code[offset + 1];
    std::cout << std::format("{:<8} {:4d} '", name, constant);
    pool[constant].print();
    std::cout << "'\n";
    return offset + 2;
}

int Chunk::constantLongInstruction(const std::string& name, int offset)
{
    uint32_t constant = code[offset + 1] | (code[offset + 2] << 8) | (code[offset + 3] << 16);
    std::cout << std::format("{:<8} {:8d} '", name, constant);
    pool[constant].print();
    std::cout << "'\n";
    return offset + 4;
}

int Chunk::simpleInstruction(const std::string& name, int offset)
{
    std::cout << name << '\n';
    return offset + 1;
}

int Chunk::addConstant(Value value)
{
    pool.push_back(value);
    return pool.size() - 1;
}
