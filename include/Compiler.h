#pragma once
#include "Chunk.h"
#include "Instructions.h"
#include "Token.h"
#include <functional>
#include <iostream>
#include <optional>
#include <vector>

enum class Precedence {
    NONE,
    ASSIGNMENT,
    OR,
    AND,
    EQUALITY,
    COMPARISON,
    TERM,
    FACTOR,
    UNARY,
    CALL,
    PRIMARY
};

using ParseFn = std::function<void()>;

struct ParseRule {
    ParseFn prefix;
    ParseFn infix;
    Precedence precedence;
};

class Compiler {

public:
    Compiler(std::vector<Token>& tokens)
        : tokens { std::move(tokens) }
    {
    }
    Compiler() = default;

    size_t getLine()
    {
        return current;
    }

private:
    Chunk currentChunk { 100 };
    std::vector<Token> tokens;
    bool hadError = false;
    size_t current = 0;
    static void report(const int line, const std::string& where, const std::string& message)
    {
        std::cerr << "[line " << line << "] Error " << where << ": " << message << std::endl;
    }
    void handleError(const int line, const std::string& message)
    {
        report(line, "", message);
        hadError = true;
    }
    void synchronize()
    {
        advance();
        while (!isAtEnd()) {
            if (previous().type == Tokentype::SEMICOLON)
                return;

            switch (peek().type) {
            case Tokentype::CLASS:
            case Tokentype::FUN:
            case Tokentype::LET:
            case Tokentype::FOR:
            case Tokentype::IF:
            case Tokentype::WHILE:
            case Tokentype::PRINT:
            case Tokentype::RETURN:
                return;
            }

            advance();
        }
    }

    bool check(Tokentype type)
    {
        if (isAtEnd())
            return false;
        return tokens[current].type == type;
    }

    Token& peek()
    {
        return tokens.at(current);
    }

    Token& previous()
    {
        return tokens.at(current - 1);
    }
    Token& advance()
    {
        if (!isAtEnd()) {
            current++;
        }
        return previous();
    }

    std::optional<Token> consume(const Tokentype type, const std::string& message)
    {
        if (check(type))
            return std::make_optional<Token>(advance());
        handleError(current, message);
        return std::nullopt;
    }

    void grouping()
    {
        expression();
        consume(Tokentype::RIGHTPEREN, "Expect ')' after expression.");
    }

    bool isAtEnd()
    {
        return peek().type == Tokentype::EOF_TOKEN;
    }

    void emitByte(uint8_t byte)
    {
        currentChunk.writeChunk(byte, current);
    }

    void emitBytes(uint8_t byte1, uint8_t byte2)
    {
        emitByte(byte1);
        emitByte(byte2);
    }
    void emitReturn()
    {
        emitByte(OP_CODE::RETURN);
    }
    void emitConstant(double value)
    {
        currentChunk.writeConstant(value, current);
    }

    void number()
    {
        double value = std::stod(previous().lexeme);
        emitConstant(value);
    }
};
