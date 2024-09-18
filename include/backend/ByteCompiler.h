#pragma once

#include "Chunk.h"
#include "Expression.h"
#include "Object.h"
#include "ScopeManager.h"
#include "Statement.h"
#include "Token.h"
#include "Value.h"
#include <memory>
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

    void pushFunction(const Token name);
    void compile(Statement& stmt);
    void compile(Expression& expr);

    /* ------ Statement compilation functions ------*/
    void compileExpressionStatement(ExpressionStatement& e);
    void compilePrintStatment(PrintStatement& p);
    void compileVariableDeclaration(const VariableDeclaration& v);
    void compileBlockStatement(BlockStatement& b);
    void compileIfStatement(IfStatement& i);
    void compileWhileStatement(WhileStatement& w);
    void compileForStatement(ForStatement& f);
    void compileReturnStatement(ReturnStatement& r);
    void compileBreakStatement(BreakStatement& b);
    void compileContinueStatment(ContinueStatement& c);
    void compileFunctionDeclaration(FunctionDeclaration& f);
    void compileSwitchStatement(const SwitchStatement& s);

    /* ------ Expression compilation functions ------*/
    void compileLiteral(const LiteralExpression& l);
    void compileVariable(const VariableExpression& v);
    void compileUnary(const UnaryExpression& u);
    void compileBinary(const BinaryExpression& b);
    void compileAssignment(const AssignmentExpression& a);
    void compileLogical(const LogicalExpression& l);
    void compileCall(const CallExpression& c);

    /* ------ Helper functions ------*/
    void function(FunctionDeclaration& f);
    ObjFunction* endCompiler();
    void emitByte(uint8_t byte);
    void emitBytes(uint8_t byte1, uint8_t byte2);
    void emitLoop(int loopStart);
    int emitJump(uint8_t instruction);
    void patchJump(int offset);
    void emitReturn();
    uint8_t makeConstant(Value value);
    uint8_t identifierConstant(const Token& name);
    void defineVariable(uint8_t global);
    void markInitialized();
    void markInitialized(ScopeManager::Variable& variable);
    void emitGetVariable(const ScopeManager::Variable& var);
    void emitSetVariable(const ScopeManager::Variable& var);
    void beginScope();
    void endScope();
    int resolveUpvalue(const Token& name);
    int addUpvalue(uint8_t index, bool isLocal);
    Chunk& currentChunk();
    ObjFunction* currentFunction();
    Value makeString(const std::string& s);
    void emitConstant(Value value);

    /* ------ Error handling ------*/

    bool hadError;
    void errorAt(const Token& token, const std::string& message);
    void error(const std::string& message);
};
