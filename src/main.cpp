#include <cmath>
#include <cstdint>
#include <cstdio>
#include <iostream>
#include <vector>
enum OPCODE { OP_RETURN, CONSTANT };

struct Chunk {
  std::vector<std::uint8_t> code;
  std::vector<std::double_t> constants;
  Chunk(size_t chunkSize = 0) {
    code = {};
    code.resize(chunkSize);
    constants = {};
    constants.resize(chunkSize);
  }

  void writeChunk(std::uint8_t byte) { code.push_back(byte); }

  void disassembleChunk() {
    for (int i = 0; i < code.size(); i++) {
      disassembleInstruction(i);
    }
  }

  int addConstant(std::double_t value) {
    constants.push_back(value);
    return constants.size() - 1;
  }

private:
  void disassembleInstruction(int offset) {
    std::printf("---- %04d offset ----\n", offset);
    uint8_t instruction = code[offset];

    switch (instruction) {
    case OPCODE::OP_RETURN:
      std::cout << "OP_RETURN" << std::endl;
      break;
    case OPCODE::CONSTANT:
      std::cout << "CONSTANT" << std::endl;
      break;
    default:
      std::printf("INVALID OPCODE %04d", instruction);
    }
  }
};

int main(int argv, char *argc[]) {
  auto chunk = Chunk{};
  chunk.writeChunk(OP_RETURN);
  chunk.disassembleChunk();
};
