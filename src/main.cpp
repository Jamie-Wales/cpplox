#include <cmath>
#include <cstdint>
#include <cstdio>
#include <initializer_list>
#include <iostream>
#include <vector>
enum OPCODE { OP_RETURN, OP_CONSTANT, OP_CONSTANT_LONG };

struct constant {
  double value;
  void print() { std::printf("%g", value); }
};

struct lineStart {
  int start;
  int lineNumber;
};

class Chunk {
public:
  std::vector<std::uint8_t> code;
  std::vector<constant> pool;
  std::vector<lineStart> lines;
  Chunk(size_t chunkSize = 0) {
    code = {};
    code.reserve(chunkSize);
    pool = {};
    pool.reserve(chunkSize);
    lines = {};
    lines.reserve(chunkSize);
  }
  void writeConstant(double value, int line) {
    int index = addConstant({value});
    if (index < 256) {
      writeChunk(OP_CONSTANT, line);
      writeChunk(index, line);
    } else {
      writeChunk(OP_CONSTANT_LONG, line);
      writeChunk((index & 0xff), line);
      writeChunk(((index >> 8) & 0xff), line);
      writeChunk(((index >> 16) & 0xff), line);
    }
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
    for (auto &line : lines) {
      if (offset == 0 || lines[offset].start == offset) {
        std::printf("%4d ", lines[offset].lineNumber);
        return;
      } else {
        std::printf("   | ");
        return;
      }
    }
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
    std::printf("%-8s %4d '", name.c_str(), constant);
    pool[constant].print();
    std::cout << "'" << std::endl;
    return offset + 2;
  }

  int constantLongInstruction(std::string name, int offset) {
    uint16_t constant = code[offset + 1] + code[offset + 2];
    std::printf("%-8s %8d '", name.c_str(), constant);
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
  auto chunk = Chunk{100};
  chunk.writeConstant(1000, 0);
  chunk.writeConstant(2500, 1);
  chunk.writeChunk(OP_RETURN, 3);
  chunk.disassembleChunk();
};
