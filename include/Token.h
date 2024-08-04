#pragma once
#include <string>

enum class Tokentype {
    FLOAT,
    INTEGER,
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
    CARRIGERETURN,
    WHITESPACE,
    EOF_TOKEN,
    UNKNOWN
};
struct Token {
    Tokentype type;
    std::string lexeme;
    int line;
    int column;

    Token(Tokentype type, std::string lexeme, int line, int column)
        : type(type)
        , lexeme(std::move(lexeme))
        , line(line)
        , column(column)
    {
    }

    Token() = default;
};
