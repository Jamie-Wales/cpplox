#include <cmath>
#include <cstdint>
#include <cstdio>
#include <initializer_list>
#include <iostream>
#include <vector>
enum OPCODE { OP_RETURN, OP_CONSTANT, OP_CONSTANT_LONG };

struct constant {
  std::double_t value;
  void print() { std::printf("%g", value); }
};

struct lineStart {
  int start;
  int lineNumber;
};

struct Chunk {
  std::vector<std::uint8_t> code;
  std::vector<constant> pool;
  std::vector<lineStart> lines;
  Chunk(size_t chunkSize = 0) {
    code = {};
    code.resize(chunkSize);
    pool = {};
    pool.resize(chunkSize);
    lines = {};
    lines.resize(chunkSize);
  }
  void writeChunk(std::uint8_t byte, int line) {
    code.push_back(byte);
    auto itr = std::find_if(lines.begin(), lines.end(), [line](auto &element) {
      return element.lineNumber == line;
    });
    if (itr == lines.end()) {
      int size = code.size() - 1;
      lines.push_back({size, line});
    }
  }

  void writeChunk(std::initializer_list<std::uint8_t> instruction, int line) {
    for (auto &byte : instruction) {
      writeChunk(byte, line);
    }
  }

  void disassembleChunk() {
    int i = 0;
    while (i < code.size()) {
      i = disassembleInstruction(i);
    }
  }
  int addConstant(constant value) {
    pool.push_back(value);
    return pool.size() - 1;
  }

private:
  void printLineNumber(int offset) {
    for (auto &line : lines)
      if (offset == 0 || lines[offset].start == offset)
        std::printf("%4d ", lines[offset].lineNumber);
      else
        std::printf("   | ");
  }

  int disassembleInstruction(int offset) {
    std::printf("%04d offset", offset);
    printLineNumber(offset);
    uint8_t instruction = code[offset];
    switch (instruction) {
    case OPCODE::OP_RETURN:
      return simpleInstruction("OP_RETURN", offset);
    case OPCODE::OP_CONSTANT:
      return constantInstruction("constantInstruction", offset);
    case OP_CONSTANT_LONG:
      return constantLongInstruction("constantLongInstruction", offset);
    default:
      std::printf("INVALID OPCODE %04d", instruction);
      return code.size();
    }
  }

  int constantInstruction(std::string name, int offset) {
    uint8_t constant = code[offset + 1];
    std::printf("%-16s %4d '", name.c_str(), constant);
    pool[constant].print();
    std::cout << "'" << std::endl;
    return offset + 2;
  }

  int constantLongInstruction(std::string name, int offset) {
    uint8_t constant = code[offset + 1];
    std::printf("%-16s %4d '", name.c_str(), constant);
    pool[constant].print();
    std::cout << "'" << std::endl;
    return offset + 3;
  }

  int simpleInstruction(std::string name, int offset) {
    std::cout << name << std::endl;
    return offset + 1;
  }
};

int main(int argv, char *argc[]) {
  auto chunk = Chunk{};
  constant c = {1.2};
  int index = chunk.addConstant(c);
  chunk.writeChunk(OP_CONSTANT, 123);
  chunk.writeChunk(index, 123);
  chunk.writeChunk(OP_CONSTANT_LONG, 124);
  constant c1 = {100};
  constant c2 = {200};
  int cons1 = chunk.addConstant(c1);
  int cons2 = chunk.addConstant(c2);
  chunk.writeChunk(
      {static_cast<unsigned char>(cons1), static_cast<unsigned char>(cons2)},
      123);
  chunk.writeChunk(OP_RETURN, 124);
  chunk.disassembleChunk();
};
