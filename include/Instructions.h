#pragma once
#include <cstdint>

enum class OP_CODE {
    ADD,
    MULT,
    PRINT,
    LESS_EQUAL,
    GREATER_EQUAL,
    DEFINE_GLOBAL,
    SET_GLOBAL,
    POP,
    SWAP,
    DUP,
    DIV,
    NEG,
    NIL,
    TRUE,
    FALSE,
    CONSTANT,
    CONSTANT_LONG,
    EQUAL,
    GREATER,
    LESS,
    NOT,
    RETURN,
    GET_GLOBAL,
    GET_LOCAL,
    SET_LOCAL,
    JUMP_IF_FALSE,
    JUMP,
    LOOP
};

constexpr inline uint8_t cast(OP_CODE code)
{
    return static_cast<uint8_t>(code);
}

constexpr inline OP_CODE cast(uint8_t value)
{
    return static_cast<OP_CODE>(value);
}
