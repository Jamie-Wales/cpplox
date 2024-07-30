#pragma once
#include <cstdio>

enum OPCODE {
    OP_RETURN,
    OP_CONSTANT,
    OP_CONSTANT_LONG
};

struct constant {
    double value;
    void print() { std::printf("%g", value); }
};

struct lineStart {
    int start;
    int lineNumber;
};
