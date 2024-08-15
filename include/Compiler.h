#pragma once
#include "Chunk.h"
#include "Instructions.h"
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
    using ParseFn = void (Compiler::*)(bool canAssign);
    struct ParseRule {
        ParseFn prefix;
        ParseFn infix;
        Precedence precedence;
    };
    std::unordered_map<std::string, int> stringConstants;
    Chunk currentChunk { 100 };
    const std::vector<Token>& tokens;
    bool hadError = false;
    bool panicMode = false;
    size_t current;
    Token previous;
    std::unordered_map<Tokentype, ParseRule> rules;

    /* ---- Parsing ---- */
    void expression();
    void grouping(bool canAssign);
    void unary(bool canAssign);
    void binary(bool canAssign);
    void literal(bool canAssign);
    bool check(Tokentype type);
    void printStatement();
    void expressionStatement();
    void statement();
    void defineVariable(uint8_t global);
    void letDeclaration();
    void declaration();
    void variable(bool canAssign);
    void namedVariable(const Token& name, bool canAssign);
    /* ---- Emit Functions ---- */
    void emitByte(uint8_t byte);
    void emitBytes(uint8_t byte1, uint8_t byte2);
    void emitReturn();
    uint8_t parseVariable(const std::string& errorMessage);
    uint8_t identifierConstant(const Token& token);
    uint8_t emitConstant(const Value& value);
    void endCompiler();
    /* ---- Make Functiones ---- */
    OP_CODE makeConstant(Value value);
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
