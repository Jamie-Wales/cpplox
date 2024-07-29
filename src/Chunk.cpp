#include "Chunk.h"
#include <cstdio>
#include <iostream>
#include <string>
#include <sys/_types/_u_int8_t.h>

void Chunk::writeConstant(double value, int line) {
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
void Chunk::writeChunk(u_int8_t byte, int line) {
  code.push_back(byte);
  auto itr = std::find_if(lines.begin(), lines.end(), [line](auto &element) {
    return element.lineNumber == line;
  });
  if (itr == lines.end()) {
    int size = code.size() - 1;
    lines.push_back({size, line});
  }
}

int Chunk::disassembleInstruction(int offset) {
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

int Chunk::constantInstruction(std::string name, int offset) {
  uint8_t constant = code[offset + 1];
  std::printf("%-8s %4d '", name.c_str(), constant);
  pool[constant].print();
  std::cout << "'" << std::endl;
  return offset + 2;
}

int Chunk::constantLongInstruction(std::string name, int offset) {
  uint16_t constant = code[offset + 1] + code[offset + 2];
  std::printf("%-8s %8d '", name.c_str(), constant);
  pool[constant].print();
  std::cout << "'" << std::endl;
  return offset + 3;
}

int Chunk::simpleInstruction(std::string name, int offset) {
  std::cout << name << std::endl;
  return offset + 1;
};
