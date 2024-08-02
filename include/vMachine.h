#pragma once
#include "Chunk.h"
#include <stack>

enum class vState { OK, BAD };

class vMachine {
public:
  vMachine(Chunk chunk) : instructions(chunk) {}
  vMachine(vMachine &&) = default;
  vMachine(const vMachine &) = default;
  vMachine &operator=(vMachine &&) = default;
  vMachine &operator=(const vMachine &) = default;
  ~vMachine() = default;

  Value readConstant();
  Value readConstantLong();

  void run();

private:
  vState state = vState::OK;
  Chunk instructions;
  int ip = 0;
  std::stack<double> stack;
};
