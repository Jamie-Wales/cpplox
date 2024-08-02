#pragma once

#include <cstdint>
#include <string>
#include <vector>

struct Value {
  explicit Value(double value) : value(value) {}
  double value;
};

struct LineInfo {
  int offset;
  int lineNumber;
};

class Chunk {
public:
  Chunk(int size) {
    code.reserve(size);
    pool.reserve(size);
    lines.reserve(size);
  };

  std::vector<uint8_t> code;
  std::vector<Value> pool;
  std::vector<LineInfo> lines;

  void writeConstant(double value, int line);
  void writeChunk(uint8_t byte, int line);
  int disassembleInstruction(int offset);
  int constantInstruction(const std::string &name, int offset);
  int constantLongInstruction(const std::string &name, int offset);
  int simpleInstruction(const std::string &name, int offset);

private:
  int addConstant(Value value);
  void printLineNumber(int offset);
  void printValue(double value);
};
