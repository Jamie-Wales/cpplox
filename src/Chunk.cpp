#include "Chunk.h"
#include "Instructions.h"
#include <cstdio>
#include <iostream>
#include <string>

void Chunk::writeConstant(double value, int line) {
  int index = addConstant(Value{value});
  if (index < 256) {
    writeChunk(CONSTANT, line);
    writeChunk(index, line);
  } else {
    writeChunk(CONSTANT_LONG, line);
    writeChunk((index & 0xff), line);
    writeChunk(((index >> 8) & 0xff), line);
    writeChunk(((index >> 16) & 0xff), line);
  }
}

void Chunk::writeChunk(uint8_t byte, int line) {
  code.push_back(byte);
  auto itr =
      std::find_if(lines.begin(), lines.end(), [line](const LineInfo &element) {
        return element.lineNumber == line;
      });
  if (itr == lines.end()) {
    int size = code.size() - 1;
    lines.push_back({size, line});
  }
}

int Chunk::disassembleInstruction(int offset) {
  std::printf("%04d ", offset);
  printLineNumber(offset);
  uint8_t instruction = code[offset];
  switch (instruction) {
  case RETURN:
    return simpleInstruction("OP_RETURN", offset);
  case CONSTANT:
    return constantInstruction("OP_CONSTANT", offset);
  case CONSTANT_LONG:
    return constantLongInstruction("OP_CONSTANT_LONG", offset);
  default:
    std::printf("Unknown opcode %d\n", instruction);
    return offset + 1;
  }
}

void Chunk::printLineNumber(int offset) {
  if (offset > 0 && lines[offset].lineNumber == lines[offset - 1].lineNumber) {
    std::printf("   | ");
  } else {
    std::printf("%4d ", lines[offset].lineNumber);
  }
}
int Chunk::constantInstruction(const std::string &name, int offset) {
  uint8_t constant = code[offset + 1];
  std::printf("%-8s %4d '", name.c_str(), constant);
  printValue(pool[constant].value);
  std::cout << "'" << std::endl;
  return offset + 2;
}

int Chunk::constantLongInstruction(const std::string &name, int offset) {
  uint32_t constant =
      code[offset + 1] | (code[offset + 2] << 8) | (code[offset + 3] << 16);
  std::printf("%-8s %8d '", name.c_str(), constant);
  printValue(pool[constant].value);
  std::cout << "'" << std::endl;
  return offset + 4;
}

int Chunk::simpleInstruction(const std::string &name, int offset) {
  std::cout << name << std::endl;
  return offset + 1;
}

int Chunk::addConstant(Value value) {
  pool.push_back(value);
  return pool.size() - 1;
}

void Chunk::printValue(double value) { std::printf("%g", value); }
