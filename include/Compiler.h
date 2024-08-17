#pragma once
#include "Chunk.h"
#include "Instructions.h"
#include "Token.h"
#include <climits>
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

struct Local {
    Token* token;
    int scopeDepth;
    bool initialized;
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
    /* ---- Fields ---- */
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
    bool check(Tokentype type) const;
    void printStatement();
    void expressionStatement();
    void statement();
    void defineVariable(uint8_t global);
    void letDeclaration();
    void declaration();
    void variable(bool canAssign);
    void namedVariable(Token& name, bool canAssign);
    /* ---- Emit Functions ---- */
    void emitByte(uint8_t byte);
    void emitBytes(uint8_t byte1, uint8_t byte2);
    void emitReturn();
    uint8_t parseVariable(const std::string& errorMessage);
    uint8_t identifierConstant(Token& token);
    uint8_t emitConstant(const Value& value);
    void endCompiler();
    /* ---- Make Functiones ---- */
    OP_CODE makeConstant(Value value);

    static Value makeString(const std::string& s);
    bool identifiersEqual(const Token* a, const Token* b)
    {
        return a->lexeme == b->lexeme;
    }
    void beginScope()
    {
        scope++;
    }

    void endScope()
    {
        scope--;
        while (!locals.empty() && locals.back().scopeDepth > scope) {
            emitByte(cast(OP_CODE::POP));
            locals.pop_back();
        }
    }
    void declareVariable()
    {
        if (scope == 0)
            return;

        Token* name = &previous;
        for (int i = locals.size() - 1; i >= 0; i--) {
            Local* local = &locals[i];
            if (local->scopeDepth != -1 && local->scopeDepth < scope) {
                break;
            }
            if (local->token->lexeme == name->lexeme) {
                error("Already a variable with this name in this scope.");
            }
        }
        addLocal(*name);
    }
    void addLocal(Token name)
    {
        Local local;
        local.token = &name;
        local.scopeDepth = -1;
        locals.push_back(local);
    }

    void markInitialized()
    {
        if (scope == 0)
            return;
        locals.back().scopeDepth = scope;
    }
    int resolveLocal(const Token& name)
    {
        for (int i = locals.size() - 1; i >= 0; i--) {
            Local* local = &locals[i];
            if (name.lexeme == local->token->lexeme) {
                if (local->scopeDepth == -1) {
                    error("Can't read local variable in its own initializer.");
                }
                return i;
            }
        }
        return -1;
    }

    void block()
    {
        while (!check(Tokentype::RIGHTBRACE) && !check(Tokentype::EOF_TOKEN)) {
            declaration();
        }

        consume(Tokentype::RIGHTBRACE, "Expect '}' after block.");
    }
    /* ---- Helper Functions ---- */
    void parsePrecedence(Precedence precedence);
    bool match(const Tokentype& type);
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
