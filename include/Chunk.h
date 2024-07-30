#pragma once
#include "Instructions.h"
#include <cstddef>
#include <cstdio>
#include <vector>

class Chunk {
public:
    std::vector<u_int8_t> code;
    std::vector<constant> pool;
    Chunk() = default;
    Chunk(Chunk&&) = default;
    Chunk(const Chunk&) = default;
    Chunk& operator=(Chunk&&) = default;
    Chunk& operator=(const Chunk&) = default;
    ~Chunk() = default;

    Chunk(size_t chunkSize)
    {
        code = {};
        code.reserve(chunkSize);
        pool = {};
        pool.reserve(chunkSize);
        lines = {};
        lines.reserve(chunkSize);
    }
    void writeConstant(double value, int line);
    void writeChunk(u_int8_t byte, int line);
    void disassembleChunk()
    {
        int i = 0;
        while (i < code.size()) {
            i = disassembleInstruction(i);
        }
    }

private:
    std::vector<lineStart> lines;
    /* #TODO still fix me */
    void printLineNumber(u_int8_t offset)
    {
        for (auto& line : lines) {
            if (offset == 0 || lines[offset].start == offset) {
                std::printf("%4d ", lines[offset].lineNumber);
                return;
            } else {
                std::printf("   | ");
                return;
            }
        }
    }

    int addConstant(constant value)
    {
        pool.push_back(value);
        return pool.size() - 1;
    }

    int disassembleInstruction(int offset);
    int constantInstruction(std::string name, int offset);
    int constantLongInstruction(std::string name, int offset);
    int simpleInstruction(std::string name, int offset);
};
