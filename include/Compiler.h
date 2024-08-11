#pragma once
#include "Chunk.h"
#include "Instructions.h"
#include "Token.h"
#include <optional>
#include <unordered_map>
#include <vector>
#define DEBUG_PRINT_CODE

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

class Compiler {
public:
    Compiler(const std::vector<Token>& tokens)
        : tokens(tokens)
        , current(0)
    {
        initRules();
    }
    std::optional<Chunk> compile();

private:
    using ParseFn = void (Compiler::*)();

    struct ParseRule {
        ParseFn prefix;
        ParseFn infix;
        Precedence precedence;
    };

    Chunk currentChunk { 100 };
    const std::vector<Token>& tokens;
    bool hadError = false;
    bool panicMode = false;
    size_t current;
    Token previous;
    std::unordered_map<Tokentype, ParseRule> rules;

    Value makeString(const std::string& s);
    void compiler();
    ParseRule getRule(Tokentype type);
    void initRules();
    void advance();
    void consume(Tokentype type, const std::string& message);
    void errorAtCurrent(const std::string& message);
    void error(const std::string& message);
    void errorAt(const Token& token, const std::string& message);
    void endCompiler();
    void expression();
    void parsePrecedence(Precedence precedence);
    void grouping();
    void unary();
    void binary();
    void literal();
    void emitByte(uint8_t byte);
    void emitBytes(uint8_t byte1, uint8_t byte2);
    void emitReturn();
    void emitConstant(const Value& value);
    bool check(Tokentype type)
    {
        return tokens[current].type == type;
    }
    void printStatement()
    {
        expression();
        consume(Tokentype::SEMICOLON, "Expect ';' after value.");
        emitByte(OP_CODE::PRINT);
    }
    void expressionStatement()
    {
        expression();
        consume(Tokentype::SEMICOLON, "Expect ';' after expression.");
        emitByte(OP_CODE::POP);
    }
    void statement()
    {
        if (match(Tokentype::PRINT)) {
            printStatement();
        } else {
            expressionStatement();
        }
    }
    void declaration()
    {
        if (panicMode)
            synchronize();
        statement();
    }

    bool match(Tokentype type)
    {
        if (!check(type))
            return false;
        advance();
        return true;
    }
    void synchronize()
    {
        panicMode = false;

        while (tokens[current].type != Tokentype::EOF_TOKEN) {
            if (previous.type == Tokentype::SEMICOLON)
                return;
            switch (tokens[current].type) {
            case Tokentype::CLASS:
            case Tokentype::FUN:
            case Tokentype::LET:
            case Tokentype::FOR:
            case Tokentype::IF:
            case Tokentype::WHILE:
            case Tokentype::PRINT:
            case Tokentype::RETURN:
                return;

            default:;
            }

            advance();
        }
    }
};
