#include "ByteCompiler.h"
#include "Chunk.h"
#include "Expression.h"
#include "Instructions.h"
#include "Object.h"
#include "ScopeManager.h"
#include "Statement.h"
#include "Stringinterner.h"
#include "Visit.h"
#include <cstdint>
#include <iostream>
#include <memory>
#include <stdexcept>

void ByteCompiler::pushFunction(const Token name)
{
    auto fun = new ObjFunction { name.lexeme, 0, {} };
    functions.push_back(fun);
}

ObjFunction* ByteCompiler::compile(std::vector<std::unique_ptr<Statement>>& stmts)
{
    for (auto& stmt : stmts) {
        compile(*stmt);
    }

    emitReturn();
    ObjFunction* function = endCompiler();

    return hadError ? nullptr : function;
}

void ByteCompiler::compile(Statement& stmt)
{
    std::visit(overloaded {
                   [this](ExpressionStatement& e) -> void { compileExpressionStatement(e); },
                   [this](PrintStatement& p) -> void { compilePrintStatment(p); },
                   [this](VariableDeclaration& v) -> void { compileVariableDeclaration(v); },
                   [this](BlockStatement& b) -> void { compileBlockStatement(b); },
                   [this](IfStatement& i) -> void { compileIfStatement(i); },
                   [this](WhileStatement& w) -> void { compileWhileStatement(w); },
                   [this](ForStatement& f) -> void { compileForStatement(f); },
                   [this](ReturnStatement& r) -> void { compileReturnStatement(r); },
                   [this](BreakStatement& b) -> void { compileBreakStatement(b); },
                   [this](ContinueStatement& c) -> void { compileContinueStatment(c); },
                   [this](FunctionDeclaration& f) -> void { compileFunctionDeclaration(f); },
                   [this](SwitchStatement& s) -> void { compileSwitchStatement(s); },
                   [](const auto& undefined) {
                       throw std::runtime_error("Undefined Statement");
                   } },
        stmt.as);
}

void ByteCompiler::compileExpressionStatement(ExpressionStatement& e)
{
    compile(*e.expression);
}

void ByteCompiler::compilePrintStatment(PrintStatement& p)
{
    compile(*p.expression);
    emitByte(cast(OP_CODE::PRINT));
}

void ByteCompiler::compileVariableDeclaration(const VariableDeclaration& v)
{
    auto variable = scopeManager.declareVariable(v.name, v.isConst);

    if (v.initializer) {
        compile(*v.initializer);
    } else {
        emitByte(cast(OP_CODE::NIL));
    }
    scopeManager.markInitialized(variable);
    if (variable.type == ScopeManager::Variable::Type::Global) {
        uint8_t index = identifierConstant(v.name);
        emitBytes(cast(OP_CODE::DEFINE_GLOBAL), index);
    }
}

void ByteCompiler::compileBlockStatement(BlockStatement& b)
{
    beginScope();
    for (auto& stmt : b.statements) {
        compile(*stmt);
    }
    endScope();
}

void ByteCompiler::compileIfStatement(IfStatement& i)
{

    compile(*i.condition);
    int thenJump = emitJump(cast(OP_CODE::JUMP_IF_FALSE));
    emitByte(cast(OP_CODE::POP));
    compile(*i.thenBranch);

    int elseJump = emitJump(cast(OP_CODE::JUMP));

    patchJump(thenJump);
    emitByte(cast(OP_CODE::POP));

    if (i.elseBranch) {
        compile(*i.elseBranch);
    }
    patchJump(elseJump);
}

void ByteCompiler::compileWhileStatement(WhileStatement& w)
{
    int loopStart = currentChunk().code.size();
    compile(*w.condition);
    int exitJump = emitJump(cast(OP_CODE::JUMP_IF_FALSE));
    emitByte(cast(OP_CODE::POP));
    compile(*w.body);
    emitLoop(loopStart);
    patchJump(exitJump);
    emitByte(cast(OP_CODE::POP));
}

void ByteCompiler::compileForStatement(ForStatement& f)
{
    beginScope();

    if (f.initializer) {
        compile(*f.initializer);
    }

    int loopStart = currentChunk().code.size();
    int exitJump = -1;

    if (f.condition) {
        compile(*f.condition);
        exitJump = emitJump(cast(OP_CODE::JUMP_IF_FALSE));
        emitByte(cast(OP_CODE::POP));
    }

    if (f.increment) {
        int bodyJump = emitJump(cast(OP_CODE::JUMP));
        int incrementStart = currentChunk().code.size();
        compile(*f.increment);
        emitByte(cast(OP_CODE::POP));
        emitLoop(loopStart);
        loopStart = incrementStart;
        patchJump(bodyJump);
    }

    compile(*f.body);
    emitLoop(loopStart);

    if (exitJump != -1) {
        patchJump(exitJump);
        emitByte(cast(OP_CODE::POP));
    }

    endScope();
}

void ByteCompiler::compileReturnStatement(ReturnStatement& r)
{
    if (functions.back()->name == "Main") {
        errorAt(r.keyword, "Can't return from top-level code.");
    }

    if (r.value) {
        compile(*r.value);
    } else {
        emitByte(cast(OP_CODE::NIL));
    }

    emitByte(cast(OP_CODE::RETURN));
}

void ByteCompiler::compileBreakStatement(BreakStatement& br)
{
    errorAt(br.keyword, "Break can only be used inside a loop");
}

void ByteCompiler::compileContinueStatment(ContinueStatement& c)
{
    errorAt(c.keyword, "Continue can only be used inside a loop");
}

void ByteCompiler::compileFunctionDeclaration(FunctionDeclaration& f)
{
    // Declare the function in the current scope
    auto variable = scopeManager.declareVariable(f.name, false);
    markInitialized(variable);

    // Compile the function
    function(f);

    // If it's a global function, emit the bytecode to define it
    if (variable.type == ScopeManager::Variable::Type::Global) {
        uint8_t global = identifierConstant(f.name);
        emitBytes(cast(OP_CODE::DEFINE_GLOBAL), global);
    }
}

void ByteCompiler::function(FunctionDeclaration& f)
{
    pushFunction(f.name);
    beginScope();

    for (const auto& param : f.parameters) {
        auto variable = scopeManager.declareVariable(param, false);
        markInitialized(variable);
    }

    compile(*f.body);
    const auto compiledFunction = endCompiler();
    emitBytes(cast(OP_CODE::CLOSURE), makeConstant(Value(compiledFunction)));

    for (int i = 0; i < compiledFunction->upValueCount; i++) {
        emitByte(upvalues[i].isLocal ? 1 : 0);
        emitByte(upvalues[i].index);
    }
}

void ByteCompiler::compileSwitchStatement(const SwitchStatement& s)
{
    compile(*s.expression);
    std::vector<int> endJumps;
    bool hasDefault = false;

    for (const auto&[fst, snd] : s.cases) {
        if (fst) {
            compile(*fst);
            emitByte(cast(OP_CODE::EQUAL));
            const int caseJump = emitJump(cast(OP_CODE::JUMP_IF_FALSE));
            emitByte(cast(OP_CODE::POP));
            compile(*snd);
            endJumps.push_back(emitJump(cast(OP_CODE::JUMP)));
            patchJump(caseJump);
            emitByte(cast(OP_CODE::POP));
        } else {
            if (hasDefault) {
                error("Cannot have multiple default cases in switch statement.");
            }
            hasDefault = true;
            compile(*snd);
        }
    }

    if (!hasDefault) {
        emitByte(cast(OP_CODE::POP));
    }
    for (const int jump : endJumps) {
        patchJump(jump);
    }
}

void ByteCompiler::compile(Expression& expr)
{
    std::visit(overloaded {
                   [this](const LiteralExpression& l) { compileLiteral(l); },
                   [this](const VariableExpression& v) { compileVariable(v); },
                   [this](const UnaryExpression& u) { compileUnary(u); },
                   [this](BinaryExpression& b) { compileBinary(b); },
                   [this](const AssignmentExpression& a) { compileAssignment(a); },
                   [this](LogicalExpression& lo) { compileLogical(lo); },
                   [this](CallExpression& c) { compileCall(c); } },
        expr.as);
}

/* Expression compilation functions */

void ByteCompiler::compileLiteral(const LiteralExpression& l)
{
    switch (l.value.type) {
    case Tokentype::TRUE:
        emitByte(cast(OP_CODE::TRUE));
        break;
    case Tokentype::FALSE:
        emitByte(cast(OP_CODE::FALSE));
        break;
    case Tokentype::INTEGER: {
        const double value = std::stod(l.value.lexeme);
        emitConstant(Value(value));
    } break;
    case Tokentype::STRING: {
        const std::string value = l.value.lexeme.substr(1, l.value.lexeme.length() - 2);
        emitConstant(makeString(value));
    } break;
    case Tokentype::NIL:
        emitByte(cast(OP_CODE::NIL));
        break;
    default:
        error("Unexpected literal type.");
        break;
    }
}
void ByteCompiler::compileVariable(const VariableExpression& v)
{
    const std::optional<ScopeManager::Variable> variable = scopeManager.resolveVariable(v.name);
    if (!variable) {
        error("Undefined variable '" + v.name.lexeme + "'.");
        return;
    }
    emitGetVariable(*variable);
}

void ByteCompiler::compileUnary(const UnaryExpression& u)
{
    compile(*u.operand);
    switch (u.operatorToken.type) {
    case Tokentype::MINUS:
        emitByte(static_cast<uint8_t>(OP_CODE::NEG));
        break;
    case Tokentype::BANG:
        emitByte(static_cast<uint8_t>(OP_CODE::NOT));
        break;
    default:
        throw std::logic_error("ERROR: No other Unary types");
    }
}

void ByteCompiler::compileBinary(const BinaryExpression& b)
{
    compile(*b.left);
    compile(*b.right);
    switch (const Tokentype operatorType = b.operatorToken.type) {
    case Tokentype::PLUS:
        emitByte(static_cast<uint8_t>(OP_CODE::ADD));
        break;
    case Tokentype::MINUS:
        emitBytes(static_cast<uint8_t>(OP_CODE::NEG), static_cast<uint8_t>(OP_CODE::ADD));
        break;
    case Tokentype::STAR:
        emitByte(static_cast<uint8_t>(OP_CODE::MULT));
        break;
    case Tokentype::SLASH:
        emitByte(static_cast<uint8_t>(OP_CODE::DIV));
        break;
    case Tokentype::BANG_EQUAL:
        emitBytes(static_cast<uint8_t>(OP_CODE::EQUAL), static_cast<uint8_t>(OP_CODE::NOT));
        break;
    case Tokentype::EQUAL_EQUAL:
        emitByte(static_cast<uint8_t>(OP_CODE::EQUAL));
        break;
    case Tokentype::GREATER:
        emitByte(static_cast<uint8_t>(OP_CODE::GREATER));
        break;
    case Tokentype::GREATER_EQUAL:
        emitBytes(static_cast<uint8_t>(OP_CODE::LESS), static_cast<uint8_t>(OP_CODE::NOT));
        break;
    case Tokentype::LESS:
        emitByte(static_cast<uint8_t>(OP_CODE::LESS));
        break;
    case Tokentype::LESS_EQUAL:
        emitBytes(static_cast<uint8_t>(OP_CODE::GREATER), static_cast<uint8_t>(OP_CODE::NOT));
        break;
    default:
        throw std::logic_error("invalid binary operator");
    }
}

void ByteCompiler::compileLogical(const LogicalExpression& l)
{
    compile(*l.left);
    compile(*l.right);
    switch (const Tokentype operatorType = l.operatorToken.type) {
    case Tokentype::BANG_EQUAL:
        emitBytes(static_cast<uint8_t>(OP_CODE::EQUAL), static_cast<uint8_t>(OP_CODE::NOT));
        break;
    case Tokentype::EQUAL_EQUAL:
        emitByte(static_cast<uint8_t>(OP_CODE::EQUAL));
        break;
    case Tokentype::GREATER:
        emitByte(static_cast<uint8_t>(OP_CODE::GREATER));
        break;
    case Tokentype::GREATER_EQUAL:
        emitBytes(static_cast<uint8_t>(OP_CODE::LESS), static_cast<uint8_t>(OP_CODE::NOT));
        break;
    case Tokentype::LESS:
        emitByte(static_cast<uint8_t>(OP_CODE::LESS));
        break;
    case Tokentype::LESS_EQUAL:
        emitBytes(static_cast<uint8_t>(OP_CODE::GREATER), static_cast<uint8_t>(OP_CODE::NOT));
        break;
    default:
        throw std::logic_error("Invalid binary operator");
    }
}

void ByteCompiler::compileAssignment(const AssignmentExpression& a)
{
    const auto variable = scopeManager.resolveVariable(a.name);
    if (!variable) {
        error("Undefined variable '" + a.name.lexeme + "'.");
        return;
    }

    if (variable->isReadOnly) {
        error("Cannot assign to const variable '" + a.name.lexeme + "'.");
        return;
    }

    compile(*a.value);
    emitSetVariable(*variable);
}
void ByteCompiler::compileCall(const CallExpression& c)
{
    compile(*c.callee);
    for (auto& arg : c.arguments) {
        compile(*arg);
    }
    emitBytes(cast(OP_CODE::CALL), static_cast<uint8_t>(c.arguments.size()));
}

ObjFunction* ByteCompiler::endCompiler()
{
    emitReturn();
    ObjFunction* function = functions.back();

#ifdef DEBUG_PRINT_CODE
    if (!hadError) {
        currentChunk().disassembleChunk(function->name != "" ? function->name : "<script>");
    }
#endif

    functions.pop_back();
    scopeManager.exitScope();

    return function;
}

/* ------ Helper functions ------ */

void ByteCompiler::emitByte(const uint8_t byte)
{
    currentChunk().writeChunk(byte, 0); // Use 0 as a placeholder for line number, or implement a different line tracking mechanism
}

void ByteCompiler::emitBytes(const uint8_t byte1, const uint8_t byte2)
{
    emitByte(byte1);
    emitByte(byte2);
}

void ByteCompiler::emitLoop(const int loopStart)
{
    emitByte(cast(OP_CODE::LOOP));

    int offset = currentChunk().code.size() - loopStart + 2;
    if (offset > UINT16_MAX)
        error("Loop body too large.");

    emitByte((offset >> 8) & 0xff);
    emitByte(offset & 0xff);
}

void ByteCompiler::error(const std::string& message)
{
    if (panicMode)
        return;
    panicMode = true;
    std::cerr << "Compile Error: " << message << std::endl;
    hadError = true;
}

int ByteCompiler::emitJump(const uint8_t instruction)
{
    emitByte(instruction);
    emitByte(0xff);
    emitByte(0xff);
    return currentChunk().code.size() - 2;
}

void ByteCompiler::patchJump(const int offset)
{
    const int jump = currentChunk().code.size() - offset - 2;

    if (jump > UINT16_MAX) {
        error("Too much code to jump over.");
    }

    currentChunk().code[offset] = (jump >> 8) & 0xff;
    currentChunk().code[offset + 1] = jump & 0xff;
}

void ByteCompiler::emitReturn()
{
    emitByte(cast(OP_CODE::NIL));
    emitByte(cast(OP_CODE::RETURN));
}

uint8_t ByteCompiler::makeConstant(const Value value)
{
    const int constant = currentChunk().addConstant(value);
    if (constant > UINT8_MAX) {
        error("Too many constants in one chunk.");
        return 0;
    }

    return static_cast<uint8_t>(constant);
}

uint8_t ByteCompiler::identifierConstant(const Token& name)
{
    return makeConstant(makeString(name.lexeme));
}

void ByteCompiler::errorAt(const Token& token, const std::string& message)
{
    if (panicMode)
        return;
    panicMode = true;
    std::cerr << "[line " << token.line << "] Error";
    if (token.type == Tokentype::EOF_TOKEN) {
        std::cerr << " at end";
    } else {
        std::cerr << " at '" << token.lexeme << "'";
    }
    std::cerr << ": " << message << std::endl;
}

void ByteCompiler::defineVariable(uint8_t global)
{
    if (scopeManager.scopes.size() > 1) {
        markInitialized();
        return;
    }

    emitBytes(cast(OP_CODE::DEFINE_GLOBAL), global);
}

void ByteCompiler::markInitialized()
{
    if (scopeManager.scopes.empty())
        return;
    scopeManager.markInitialized();
}

void ByteCompiler::markInitialized(ScopeManager::Variable& variable)
{
    scopeManager.markInitialized(variable);
}

void ByteCompiler::emitGetVariable(const ScopeManager::Variable& var)
{
    switch (var.type) {
    case ScopeManager::Variable::Type::Local:
        emitBytes(cast(OP_CODE::GET_LOCAL), var.index);
        break;
    case ScopeManager::Variable::Type::Upvalue:
        emitBytes(cast(OP_CODE::GET_UPVALUE), var.index);
        break;
    case ScopeManager::Variable::Type::Global:
        emitBytes(cast(OP_CODE::GET_GLOBAL), identifierConstant(var.name));
        break;
    }
}

void ByteCompiler::emitSetVariable(const ScopeManager::Variable& var)
{
    switch (var.type) {
    case ScopeManager::Variable::Type::Local:
        emitBytes(cast(OP_CODE::SET_LOCAL), var.index);
        break;
    case ScopeManager::Variable::Type::Upvalue:
        emitBytes(cast(OP_CODE::SET_UPVALUE), var.index);
        break;
    case ScopeManager::Variable::Type::Global:
        emitBytes(cast(OP_CODE::SET_GLOBAL), identifierConstant(var.name));
        break;
    }
}

void ByteCompiler::beginScope()
{
    scopeManager.enterScope();
}

void ByteCompiler::endScope()
{
    scopeManager.exitScope();
    while (currentChunk().code.size() > scopeManager.scopes.back().variables.size()) {
        emitByte(cast(OP_CODE::POP));
        currentChunk().code.pop_back();
    }
}

int ByteCompiler::resolveUpvalue(const Token& name)
{
    if (functions.size() == 1)
        return -1;

    auto variable = scopeManager.resolveVariable(name);
    if (variable) {
        if (variable->type == ScopeManager::Variable::Type::Local) {
            return addUpvalue(variable->index, true);
        } else if (variable->type == ScopeManager::Variable::Type::Upvalue) {
            return addUpvalue(variable->index, false);
        }
    }
    int upvalue = resolveUpvalue(name);
    if (upvalue != -1) {
        return addUpvalue(static_cast<uint8_t>(upvalue), false);
    }

    return -1;
}

int ByteCompiler::addUpvalue(uint8_t index, bool isLocal)
{
    int upvalueCount = currentFunction()->upValueCount;

    for (int i = 0; i < upvalueCount; i++) {
        Upvalue& upvalue = upvalues[i];
        if (upvalue.index == index && upvalue.isLocal == isLocal) {
            return i;
        }
    }

    upvalues.push_back({ index, isLocal });
    return currentFunction()->upValueCount++;
}
Chunk& ByteCompiler::currentChunk()
{
    return functions.back()->chunk;
}

ObjFunction* ByteCompiler::currentFunction()
{
    return functions.back();
}
Value ByteCompiler::makeString(const std::string& s)
{
    const std::string* internedString = StringInterner::instance().intern(s);
    return Value(new Obj(ObjString(internedString)));
}
void ByteCompiler::emitConstant(Value value) { emitBytes(cast(OP_CODE::CONSTANT), makeConstant(value)); }
