#pragma once

#include "Chunk.h"
#include "Expression.h"
#include "Object.h"
#include "ScopeManager.h"
#include "Statement.h"
#include "Token.h"
#include "Value.h"

#include <vector>
class ByteCompiler {
public:
    ByteCompiler()
        : hadError(false)
    {
        Token mainToken = { Tokentype::IDENTIFIER, "main", 0, 0 };
        pushFunction(mainToken);
    }
    ObjFunction* compile(std::vector<std::unique_ptr<Statement>>& stmts);

private:
    struct Upvalue {
        uint8_t index;
        bool isLocal;
    };

    std::vector<ObjFunction*> functions;
    ScopeManager scopeManager;
    std::vector<Upvalue> upvalues;
    bool panicMode = false;

    void pushFunction(const Token& name);
    void compile(Statement& stmt);
    void compile(Expression& expr);

    /* ------ Statement compilation functions ------*/
    void compileExpressionStatement(const ExpressionStatement& e);
    void compilePrintStatment(const PrintStatement& p);
    void compileVariableDeclaration(const VariableDeclaration& v);
    void compileBlockStatement(const BlockStatement& b);
    void compileIfStatement(const IfStatement& i);
    void compileWhileStatement(const WhileStatement& w);
    void compileForStatement(const ForStatement& f);
    void compileReturnStatement(const ReturnStatement& r);
    void compileBreakStatement(const BreakStatement& b);
    void compileContinueStatment(const ContinueStatement& c);
    void compileFunctionDeclaration(const FunctionDeclaration& f);

    Value makeFunction(ObjFunction* function);

    void compileSwitchStatement(const SwitchStatement& s);

    /* ------ Expression compilation functions ------*/
    void compileLiteral(const LiteralExpression& l);
    void compileVariable(const VariableExpression& v);
    void compileUnary(const UnaryExpression& u);
    void compileBinary(const BinaryExpression& b);
    void compileAssignment(const AssignmentExpression& a);
    void compileLogical(const LogicalExpression& l);
    void compileCall(const CallExpression& c);
    void compilePrePostfix(const IncrementExpression& i);

    /* ------ Helper functions ------*/
    void function(const FunctionDeclaration& f);
    ObjFunction* endCompiler();
    void emitByte(uint8_t byte) const;
    void emitBytes(uint8_t byte1, uint8_t byte2) const;
    void emitLoop(int loopStart);
    [[nodiscard]] int emitJump(uint8_t instruction) const;
    void patchJump(int offset);
    void emitReturn() const;
    uint8_t makeConstant(Value value);
    uint8_t identifierConstant(const Token& name);
    void defineVariable(uint8_t global);
    void markInitialized();
    void markInitialized(ScopeManager::Variable& variable) const;
    void emitGetVariable(const ScopeManager::Variable& var);
    void emitSetVariable(const ScopeManager::Variable& var);
    void beginScope();
    void endScope();
    int resolveUpvalue(const Token& name);
    int addUpvalue(uint8_t index, bool isLocal);
    [[nodiscard]] Chunk& currentChunk() const;
    [[nodiscard]] ObjFunction* currentFunction();
    Value makeString(const std::string& s);
    void emitConstant(Value value);

    /* ------ Error handling ------*/

    bool hadError;
    void errorAt(const Token& token, const std::string& message);
    void error(const std::string& message);
};
