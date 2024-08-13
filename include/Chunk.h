#pragma once

#include "Instructions.h"
#include "Value.h"
#include <cstddef>
#include <string>
#include <vector>

class Chunk {
public:
    Chunk(int size)
    {
        code.reserve(size);
        pool.reserve(size);
        lines.reserve(size);
    };

    struct LineInfo {
        int offset;
        int lineNumber;
    };

    std::vector<uint8_t> code;
    std::vector<Value> pool;
    std::vector<LineInfo> lines;

    int writeConstant(const Value&, int line);
    void writeChunk(OP_CODE byte, int line);
    void disassembleChunk(const std::string_view& name);
    int disassembleInstruction(int offset);
    int constantInstruction(const std::string& name, int offset) const;
    int constantLongInstruction(const std::string& name, int offset) const;
    int simpleInstruction(const std::string& name, int offset) const;
    int addConstant(const Value& value);

private:
    void printLineNumber(int offset) const;
};
