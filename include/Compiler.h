#pragma once
#include "Chunk.h"
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
    void emitConstant(double value);
};
