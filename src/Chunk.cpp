#include "Chunk.h"
#include "Instructions.h"
#include <format>
#include <iostream>
#include <string>

void Chunk::disassembleChunk(const std::string_view& name)
{
    std::cout << std::format("== {} ==\n", name);

    for (size_t offset = 0; offset < code.size();) {
        offset = disassembleInstruction(offset);
    }
}

int Chunk::writeConstant(const Value& value, int line)
{
    int index = addConstant(value);
    if (index < 256) {
        writeChunk(OP_CODE::CONSTANT, line);
        writeChunk(static_cast<OP_CODE>(index), line);
    } else {
        writeChunk(OP_CODE::CONSTANT_LONG, line);
        writeChunk(cast((index & 0xff)), line);
        writeChunk(cast((index >> 8) & 0xff), line);
        writeChunk(cast((index >> 16) & 0xff), line);
    }

    return index;
}
void Chunk::writeChunk(OP_CODE byte, int line)
{
    code.push_back(cast(byte));
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
    uint8_t instruction = code[offset];
    OP_CODE opcode = cast(instruction);
    std::cout << std::format("{:04d} ", offset);
    printLineNumber(offset);
    switch (instruction) {
    case cast(OP_CODE::RETURN):
        return simpleInstruction("OP_RETURN", offset);
    case cast(OP_CODE::CONSTANT):
        return constantInstruction("OP_CONSTANT", offset);
    case cast(OP_CODE::CONSTANT_LONG):
        return constantLongInstruction("OP_CONSTANT_LONG", offset);
    case cast(OP_CODE::ADD):
        return simpleInstruction("OP_ADD", offset);
    case cast(OP_CODE::MULT):
        return simpleInstruction("OP_MULTIPLY", offset);
    case cast(OP_CODE::DIV):
        return simpleInstruction("OP_DIVIDE", offset);
    case cast(OP_CODE::NEG):
        return simpleInstruction("OP_NEGATE", offset);
    case cast(OP_CODE::NIL):
        return simpleInstruction("OP_NIL", offset);
    case cast(OP_CODE::TRUE):
        return simpleInstruction("OP_TRUE", offset);
    case cast(OP_CODE::FALSE):
        return simpleInstruction("OP_FALSE", offset);
    case cast(OP_CODE::NOT):
        return simpleInstruction("OP_NOT", offset);
    case cast(OP_CODE::GREATER):
        return simpleInstruction("OP_GREATER", offset);
    case cast(OP_CODE::LESS):
        return simpleInstruction("OP_LESS", offset);
    case cast(OP_CODE::EQUAL):
        return simpleInstruction("OP_EQUAL", offset);
    case cast(OP_CODE::PRINT):
        return simpleInstruction("OP_PRINT", offset);
    case cast(OP_CODE::POP):
        return simpleInstruction("OP_POP", offset);
    case cast(OP_CODE::DEFINE_GLOBAL):
        return constantInstruction("OP_DEFINE_GLOBAL", offset);
    case cast(OP_CODE::GET_GLOBAL):
        return constantInstruction("OP_GET_GLOBAL", offset);
    default:
        std::cout << std::format("Unknown opcode {}\n", instruction);
        return offset + 1;
    }
}

void Chunk::printLineNumber(int offset) const
{
    for (const auto& lineInfo : lines) {
        if (lineInfo.offset == offset) {
            std::cout << std::format("{:4d} ", lineInfo.lineNumber);
            return;
        }
    }
    std::cout << "   | ";
}

int Chunk::constantInstruction(const std::string& name, int offset) const
{
    uint8_t constant = code[offset + 1];
    std::cout << std::format("{:<8} {:4d} '", name, constant);
    pool[constant].print();
    std::cout << "'\n";
    return offset + 2;
}

int Chunk::constantLongInstruction(const std::string& name, int offset) const
{
    uint32_t constant = code[offset + 1] | (code[offset + 2] << 8) | (code[offset + 3] << 16);
    std::cout << std::format("{:<8} {:8d} '", name, constant);
    pool[constant].print();
    std::cout << "'\n";
    return offset + 4;
}

int Chunk::simpleInstruction(const std::string& name, int offset) const
{
    std::cout << name << '\n';
    return offset + 1;
}

int Chunk::addConstant(const Value& value)
{
    pool.push_back(value);
    return pool.size() - 1;
}
