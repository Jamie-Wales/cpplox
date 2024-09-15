#include "Chunk.h"
#include "Instructions.h"
#include "Object.h"
#include <format>
#include <iostream>
#include <string>

void Chunk::disassembleChunk(const std::string& name) const
{
    std::cout << std::format("== {} ==\n", name);
    for (size_t offset = 0; offset < code.size();) {
        offset = disassembleInstruction(offset);
    }
}

int Chunk::writeConstant(const Value& value, const int line)
{
    const int index = addConstant(value);
    if (index < 256) {
        writeChunk(cast(OP_CODE::CONSTANT), line);
        writeChunk(index, line);
    } else {
        writeChunk(cast(OP_CODE::CONSTANT_LONG), line);
        writeChunk((index & 0xff), line);
        writeChunk((index >> 8) & 0xff, line);
        writeChunk((index >> 16) & 0xff, line);
    }
    return index;
}

int Chunk::disassembleJump(const std::string& name, int sign, int offset) const
{
    const uint16_t jump = (code[offset + 1] << 8) | code[offset + 2];
    std::cout << std::format("{:<16} {:4d} -> {}\n",
        name, offset, offset + 3 + sign * jump);
    return offset + 3;
}

void Chunk::writeChunk(const uint8_t byte, int line)
{
    code.push_back(byte);
    lines.push_back({ static_cast<int>(code.size() - 1), line });
}

int Chunk::disassembleInstruction(int offset) const
{
    uint8_t instruction = code[offset];
    std::cout << std::format("{:04d} ", offset);
    printLineNumber(offset);
    switch (instruction) {
    case cast(OP_CODE::CLOSURE): {
        offset++;
        uint8_t constant = code[offset++];
        std::cout << std::format("{:<16} {:>4}", "CLOSURE", constant);
        pool[constant].print();
        std::cout << std::endl;
        auto func = pool[constant].asFunc();
        std::cout << std::format("         {:<16} {}\n", "upvalue count:", func->upValueCount);
        for (size_t i = 0; i < func->upValueCount; i++) {
            uint8_t isLocal = code[offset++];
            uint8_t index = code[offset++];
            std::cout << std::format("         {:<16} | {:<7} | {}\n",
                "", isLocal ? "local" : "upvalue", index);
        }
        return offset;
    }
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
    case cast(OP_CODE::GREATER_EQUAL):
        return simpleInstruction("OP_GREATER_EQUAL", offset);
    case cast(OP_CODE::LESS_EQUAL):
        return simpleInstruction("OP_LESS_EQUAL", offset);
    case cast(OP_CODE::PRINT):
        return simpleInstruction("OP_PRINT", offset);
    case cast(OP_CODE::POP):
        return simpleInstruction("OP_POP", offset);
    case cast(OP_CODE::DEFINE_GLOBAL):
        return constantInstruction("OP_DEFINE_GLOBAL", offset);
    case cast(OP_CODE::SET_GLOBAL):
        return constantInstruction("OP_SET_GLOBAL", offset);
    case cast(OP_CODE::GET_GLOBAL):
        return constantInstruction("OP_GET_GLOBAL", offset);
    case cast(OP_CODE::SET_LOCAL):
        return byteInstruction("OP_SET_LOCAL", offset);
    case cast(OP_CODE::GET_LOCAL):
        return byteInstruction("OP_GET_LOCAL", offset);
    case cast(OP_CODE::JUMP):
        return disassembleJump("OP_JUMP", 1, offset);
    case cast(OP_CODE::CLOSE_UPVALUE):
        return simpleInstruction("CLOSE_UPVALUE", offset);
    case cast(OP_CODE::JUMP_IF_FALSE):
        return disassembleJump("OP_JUMP_IF_FALSE", 1, offset);
    case cast(OP_CODE::LOOP):
        return disassembleJump("OP_LOOP", -1, offset);
    case cast(OP_CODE::SWAP):
        return simpleInstruction("OP_SWAP", offset);
    case cast(OP_CODE::DUP):
        return simpleInstruction("OP_DUP", offset);
    case cast(OP_CODE::CALL):
        return byteInstruction("OP_CALL", offset);
    case cast(OP_CODE::GET_UPVALUE):
        return byteInstruction("GET_UP_VALUE", offset);
    case cast(OP_CODE::SET_UPVALUE):
        return byteInstruction("SET_UP_VALUE", offset);
    default:
        std::cout << std::format("Unknown opcode {}\n", instruction);
        return offset + 1;
    }
}

int Chunk::byteInstruction(const std::string& name, const int offset) const
{
    uint8_t slot = code[offset + 1];
    std::cout << std::format("{:<16} {:4d}\n", name, slot);
    return offset + 2;
}

void Chunk::printLineNumber(const int offset) const
{
    if (offset > 0 && lines[offset].lineNumber == lines[offset - 1].lineNumber) {
        std::cout << "   | ";
    } else {
        std::cout << std::format("{:4d} ", lines[offset].lineNumber);
    }
}

int Chunk::constantInstruction(const std::string& name, const int offset) const
{
    uint8_t constant = code[offset + 1];
    std::cout << std::format("{:<16} {:4d} '", name, constant);
    pool[constant].print();
    std::cout << "'\n";
    return offset + 2;
}

int Chunk::constantLongInstruction(const std::string& name, int offset) const
{
    uint32_t constant = code[offset + 1] | (code[offset + 2] << 8) | (code[offset + 3] << 16);
    std::cout << std::format("{:<16} {:8d} '", name, constant);
    pool[constant].print();
    std::cout << "'\n";
    return offset + 4;
}

int Chunk::simpleInstruction(const std::string& name, const int offset)
{
    std::cout << name << '\n';
    return offset + 1;
}

int Chunk::addConstant(const Value& value)
{
    pool.push_back(value);
    return static_cast<int>(pool.size() - 1);
}
