#pragma once
#include <string>

enum class Tokentype {
    FLOAT,
    CONST,
    INTEGER,
    BANG,
    TRUE,
    FALSE,
    STRING,
    PLUS,
    MINUS,
    LET,
    STAR,
    SLASH,
    RIGHTPEREN,
    RIGHTBRACE,
    LEFTPEREN,
    LEFTBRACE,
    SEMICOLON,
    IDENTIFIER,
    FOR,
    IF,
    ELSE,
    CLASS,
    FUN,
    WHILE,
    PRINT,
    ERROR,
    RETURN,
    CARRIGERETURN,
    WHITESPACE,
    EOF_TOKEN,
    AND,
    OR,
    NIL,
    EQUAL,
    EQUAL_EQUAL,
    BANG_EQUAL,
    LESS,
    LESS_EQUAL,
    GREATER,
    GREATER_EQUAL,
    PLUS_EQUAL,
    MINUS_EQUAL,
    STAR_EQUAL,
    SLASH_EQUAL,
    COMMENT,
    UNTERMINATED_STRING,
    UNTERMINATED_COMMENT,
    UNKNOWN
};

struct Token {
    Tokentype type;
    std::string lexeme;
    int line {};
    int column {};

    Token(const Tokentype type, std::string lexeme, const int line, const int column)
        : type(type)
        , lexeme(std::move(lexeme))
        , line(line)
        , column(column)
    {
    }

    Token() = default;
};
