#pragma once
#include "Chunk.h"
#include "Instructions.h"
#include "Token.h"
#include <climits>
#include <cstdint>
#include <optional>
#include <unordered_map>
#include <unordered_set>
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

struct Local {
    Token& token;
    int scopeDepth;
    bool isConst;
};

class Compiler {
public:
    explicit Compiler(const std::vector<Token>& tokens)
        : tokens(tokens)
        , current(0)
    {
        initRules();
    }
    std::optional<Chunk> compile();

private:
    std::unordered_set<std::string> constGlobals;
    using ParseFn = void (Compiler::*)(bool canAssign);
    struct ParseRule {
        ParseFn prefix;
        ParseFn infix;
        Precedence precedence;
    };
    int scope = 0;
    std::vector<Local> locals = {};
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
    [[nodiscard]] bool check(Tokentype type) const;
    void printStatement();
    void expressionStatement();
    void statement();
    void defineVariable(uint8_t global);
    void declaration();
    void variable(bool canAssign);
    void namedVariable(Token& name, bool canAssign);
    void variableDeclaration();
    /* ---- Emit Functions ---- */
    void emitByte(uint8_t byte);
    void emitBytes(uint8_t byte1, uint8_t byte2);
    void emitReturn();
    uint8_t parseVariable(const std::string& errorMessage);
    uint8_t identifierConstant(Token& token);
    uint8_t emitConstant(const Value& value);
    void endCompiler();
    static Value makeString(const std::string& s);
    /* ---- Helper Functions ---- */
    void parsePrecedence(Precedence precedence);
    bool match(const Tokentype& type);
    void initRules();
    void advance();
    ParseRule getRule(Tokentype type);
    bool consume(Tokentype type, const std::string& message);
    /* ---- Error Functions ---- */
    void synchronize();
    void errorAtCurrent(const std::string& message);
    void error(const std::string& message);
    void errorAt(const Token& token, const std::string& message);
    void endScope();
    void beginScope();
    void declareVariable();
    void markInitialized();
    void block();
    int resolveLocal(const Token& name);
    void addLocal(Token& name);
};
