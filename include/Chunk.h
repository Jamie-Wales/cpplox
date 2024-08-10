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

    void writeConstant(double value, int line);
    void writeChunk(uint8_t byte, int line);
    void disassembleChunk(std::string_view name)
    {
        std::cout << std::format("== {} ==\n", name);

        for (size_t offset = 0; offset < code.size();) {
            offset = disassembleInstruction(offset);
        }
    }
    int disassembleInstruction(int offset);
    int constantInstruction(const std::string& name, int offset);
    int constantLongInstruction(const std::string& name, int offset);
    int simpleInstruction(const std::string& name, int offset);

private:
    int addConstant(Value value);
    void printLineNumber(int offset);
};
