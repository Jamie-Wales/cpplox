#pragma once

#include "Chunk.h"
#include "Object.h"
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

enum class FunctionType {
    FUNCTION
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
        pushFunction({ Tokentype::IDENTIFIER, "main", 0, 0 });
        locals = {};
    }
    std::optional<ObjFunction*> compile();

private:
    struct LoopInfo {
        int start;
        int scopeDepth;
        std::vector<int> breaks;
        int continueTarget;
    };
    std::vector<LoopInfo> loopStack;
    std::vector<ObjFunction*> functions;
    void breakStatement();
    void continueStatement();
    void switchStatement();
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
    std::vector<Token> tokens;
    bool hadError = false;
    bool panicMode = false;
    size_t current;
    Token previous;
    std::unordered_map<Tokentype, ParseRule> rules;
    ObjFunction* currentFunction();
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
    void namedVariable(const Token& name, bool canAssign);
    void variableDeclaration();
    int emitJump(uint8_t instruction);
    void patchJump(int offset);
    void emitByte(uint8_t byte);
    void emitBytes(uint8_t byte1, uint8_t byte2);
    void emitReturn();
    uint8_t parseVariable(const std::string& errorMessage);
    uint8_t identifierConstant(const Token& token);
    uint8_t emitConstant(const Value& value);
    ObjFunction* endCompiler();
    void whileStatement();
    static Value makeString(const std::string& s);
    void parsePrecedence(Precedence precedence);
    bool match(const Tokentype& type);
    void initRules();
    void advance();
    ParseRule getRule(Tokentype type);
    bool consume(Tokentype type, const std::string& message);
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
    void addLocal(const Token& name);
    void forStatement();
    void funDeclaration();
    void function(FunctionType ft);
    uint8_t argumentList();
    void call(bool canAssign);
    void returnStatement();
    Value makeFunction(ObjFunction* function);
    void pushFunction(const Token& name);
    Chunk& currentChunk() const;
    void compileInto(Chunk& chunk);
};
