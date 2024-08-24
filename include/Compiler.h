#pragma once
#include "Chunk.h"
#include "Token.h"
#include <cstdint>
#include <optional>
#include <unordered_map>
#include <unordered_set>
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

struct Local {
    Token token;
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
    // #TODO look at whether its worth changing this to vector of hashmaps
    std::unordered_set<std::string> constGlobals;
    using ParseFn = void (Compiler::*)(bool canAssign);
    struct ParseRule {
        ParseFn prefix;
        ParseFn infix;
        ParseFn postfix;
        Precedence precedence;
    };
    int scope = 0;
    // #TODO look at whether its worth changing this to vector of hashmaps
    std::vector<Local> locals = {};
    std::unordered_map<std::string, int> stringConstants;
    Chunk currentChunk { 100 };
    std::vector<Token> tokens;
    bool hadError = false;
    bool panicMode = false;
    size_t current;
    Token previous;
    std::unordered_map<Tokentype, ParseRule> rules;

    /* ---- Parsing ---- */
    void emitLoop(int loopStart);
    void expression();
    void grouping(bool canAssign);
    void unary(bool canAssign);
    void binary(bool canAssign);
    void and_(bool canAssign);
    void or_(bool canAssign);
    void literal(bool canAssign);

    void prefix(bool canAssign);
    void postfix(bool canAssign);
    [[nodiscard]] bool check(Tokentype type) const;
    void printStatement();
    void expressionStatement();
    void ifStatement();
    void statement();
    void defineVariable(uint8_t global);
    void declaration();
    void variable(bool canAssign);
    void namedVariable(const Token &name, bool canAssign);
    void variableDeclaration();
    /* ---- Emit Functions ---- */
    int emitJump(uint8_t instruction);
    void patchJump(int offset);
    void emitByte(uint8_t byte);
    void emitBytes(uint8_t byte1, uint8_t byte2);
    void emitReturn();
    uint8_t parseVariable(const std::string& errorMessage);
    uint8_t identifierConstant(const Token& token);
    uint8_t emitConstant(const Value& value);
    void endCompiler();
    void whileStatement();
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
    int resolveLocal(const Token &name);
    void addLocal(const Token &name);
};
