#pragma once

#include "Value.h"
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

struct LineInfo {
    int offset;
    int lineNumber;
};

class Chunk {
public:
    Chunk(int size)
    {
        code.reserve(size);
        pool.reserve(size);
        lines.reserve(size);
    };

    std::vector<uint8_t> code;
    std::vector<Value> pool;
    std::vector<LineInfo> lines;

    void writeConstant(const Value&, int line);
    void writeChunk(uint8_t byte, int line);
    void disassembleChunk(const std::string_view& name)
    {
        std::cout << std::format("== {} ==\n", name);

        for (size_t offset = 0; offset < code.size();) {
            offset = disassembleInstruction(offset);
        }
    }
    int disassembleInstruction(int offset);
    int constantInstruction(const std::string& name, int offset) const;
    int constantLongInstruction(const std::string& name, int offset) const;
    int simpleInstruction(const std::string& name, int offset) const;

private:
    int addConstant(const Value& value);
    void printLineNumber(int offset) const;
};
