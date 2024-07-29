#include "Chunk.h"

int main(int argv, char *argc[]) {
  auto chunk = Chunk{100};
  chunk.writeConstant(1000, 0);
  chunk.writeConstant(2500, 1);
  chunk.writeChunk(OP_RETURN, 3);
  chunk.disassembleChunk();
};
