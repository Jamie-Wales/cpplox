#pragma once

#include "Instructions.h"
#include "Value.h"
#include <cstddef>
#include <string>
#include <vector>

class Chunk {
public:
    explicit Chunk(const int size)
    {
        code.reserve(size);
        pool.reserve(size);
        lines.reserve(size);
    }

    Chunk() = default;

    struct LineInfo {
        int offset;
        int lineNumber;
    };

    std::vector<uint8_t> code;
    std::vector<Value> pool;
    std::vector<LineInfo> lines;

    void disassembleChunk(const std::string_view &name) const;

    int writeConstant(const Value&, int line);
    void writeChunk(uint8_t byte, int line);
    void disassembleChunk(const std::string_view& name);
    [[nodiscard]] int disassembleInstruction(int offset) const;
    [[nodiscard]] int constantInstruction(const std::string& name, int offset) const;
    [[nodiscard]] int constantLongInstruction(const std::string& name, int offset) const;

    static int simpleInstruction(const std::string& name, int offset);
    int addConstant(const Value& value);

private:
    void printLineNumber(int offset) const;
};
