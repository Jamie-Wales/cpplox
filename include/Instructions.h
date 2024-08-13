#pragma once
#include <cstdint>

enum class OP_CODE : uint8_t {
    ADD,
    MULT,
    PRINT,
    DEFINE_GLOBAL,
    GET_GLOBAL,
    POP,
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
    RETURN
};

constexpr inline uint8_t cast(OP_CODE code)
{
    return static_cast<uint8_t>(code);
}

constexpr inline OP_CODE cast(uint8_t value)
{
    return static_cast<OP_CODE>(value);
}
