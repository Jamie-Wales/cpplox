#pragma once

enum OP_CODE { ADD,
    MULT,
    DIV,
    NEG,
    NIL,
    TRUE,
    FALSE,
    CONSTANT,
    CONSTANT_LONG,
    EQUAL,
    EQUAL_EQUAL,
    GREATER,
    LESS,
    NOT,
    RETURN };

struct constant {
    double value;
};

struct lineStart {
    int start;
    int lineNumber;
};
