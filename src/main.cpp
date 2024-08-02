#include "vMachine.h"

int main(int argc, char *argv[]) {
  auto chunk = Chunk{100};
  for (int i = 0; i < 3; i++) {
    chunk.writeConstant(i, i);
  }
  vMachine jvm{chunk};
  jvm.run();

  return 0;
}
