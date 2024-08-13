#pragma once
#include "Chunk.h"
#include "Token.h"
#include <cstdint>
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
    /* ---- Fields ---- */
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

    /* ---- Parsing ---- */
    void expression();
    void grouping();
    void unary();
    void binary();
    void literal();
    bool check(Tokentype type);
    void printStatement();
    void expressionStatement();
    void statement();
    void defineVariable(uint8_t global);
    void letDeclaration();
    void declaration();

    /* ---- Emit Functions ---- */
    void emitByte(uint8_t byte);
    void emitBytes(uint8_t byte1, uint8_t byte2);
    void emitReturn();
    uint8_t parseVariable(const std::string& errorMessage);
    uint8_t identifierConstant(const Token& token);
    uint8_t emitConstant(const Value& value);
    void endCompiler();

    /* ---- Make Functiones ---- */
    uint8_t makeConstant(Value value);
    Value makeString(const std::string& s);

    /* ---- Helper Functions ---- */
    void parsePrecedence(Precedence precedence);
    bool match(Tokentype type);
    void initRules();
    void advance();
    ParseRule getRule(Tokentype type);
    void consume(Tokentype type, const std::string& message);

    /* ---- Error Functions ---- */
    void synchronize();
    void errorAtCurrent(const std::string& message);
    void error(const std::string& message);
    void errorAt(const Token& token, const std::string& message);
};
