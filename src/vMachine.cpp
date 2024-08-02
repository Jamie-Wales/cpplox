#include "vMachine.h"
#include "Instructions.h"
#include <iostream>
#define DEBUG_TRACE_EXECUTION

Value vMachine::readConstant() {
  return instructions.pool[instructions.code[ip++]];
}

Value vMachine::readConstantLong() {
  uint32_t index = instructions.code[ip] | (instructions.code[ip + 1] << 8) |
                   (instructions.code[ip + 2] << 16);
  ip += 3;
  return instructions.pool[index];
}

void vMachine::run() {
  while (ip < instructions.code.size()) {
    uint8_t byte = instructions.code[ip++];
#ifdef DEBUG_TRACE_EXECUTION
    std::printf("          ");
    std::stack<double> tempStack = stack;
    while (!tempStack.empty()) {
      std::printf("[ ");
      printValue(tempStack.top());
      std::printf(" ]");
      tempStack.pop();
    }
    std::printf("\n");
    instructions.disassembleInstruction(ip - 1);
#endif

    switch (byte) {
    case RETURN: {
      if (!stack.empty()) {
        printValue(stack.top());
        std::cout << std::endl;
        stack.pop();
      }
      state = vState::OK;
      return;
    }
    case CONSTANT: {
      Value constant = readConstant();
      stack.push(constant.value);
      break;
    }
    case CONSTANT_LONG: {
      Value constant = readConstantLong();
      stack.push(constant.value);
      break;
    }
    default:
      std::cerr << "Unknown opcode: " << static_cast<int>(byte) << std::endl;
      return;
    }
  }
}
