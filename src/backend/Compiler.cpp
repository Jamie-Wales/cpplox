#include "Compiler.h"
#include "Chunk.h"
#include "Instructions.h"
#include "Object.h"
#include "Stringinterner.h"
#include "Token.h"
#include "Value.h"
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <optional>
#define DEBUG_PRINT_CODE
Chunk& Compiler::currentChunk() const
{
    return functions.back()->chunk;
}

void Compiler::pushFunction(const Token& name)
{
    auto fun = new ObjFunction { name.lexeme, 0, {} };
    functions.push_back(fun);
    locals.push_back({ name, scope, false });
}

std::string tokenTypeToString(const Tokentype type)
{
    switch (type) {
    case Tokentype::FLOAT:
        return "FLOAT";
    case Tokentype::INTEGER:
        return "INTEGER";
    case Tokentype::BANG:
        return "BANG";
    case Tokentype::TRUE:
        return "TRUE";
    case Tokentype::FALSE:
        return "FALSE";
    case Tokentype::STRING:
        return "STRING";
    case Tokentype::PLUS:
        return "PLUS";
    case Tokentype::MINUS:
        return "MINUS";
    case Tokentype::LET:
        return "LET";
    case Tokentype::STAR:
        return "STAR";
    case Tokentype::SLASH:
        return "SLASH";
    case Tokentype::RIGHTPEREN:
        return "RIGHTPEREN";
    case Tokentype::RIGHTBRACE:
        return "RIGHTBRACE";
    case Tokentype::LEFTPEREN:
        return "LEFTPEREN";
    case Tokentype::LEFTBRACE:
        return "LEFTBRACE";
    case Tokentype::SEMICOLON:
        return "SEMICOLON";
    case Tokentype::IDENTIFIER:
        return "IDENTIFIER";
    case Tokentype::FOR:
        return "FOR";
    case Tokentype::IF:
        return "IF";
    case Tokentype::CLASS:
        return "CLASS";
    case Tokentype::FUN:
        return "FUN";
    case Tokentype::WHILE:
        return "WHILE";
    case Tokentype::PRINT:
        return "PRINT";
    case Tokentype::ERROR:
        return "ERROR";
    case Tokentype::RETURN:
        return "RETURN";
    case Tokentype::CARRIGERETURN:
        return "CARRIGERETURN";
    case Tokentype::WHITESPACE:
        return "WHITESPACE";
    case Tokentype::EOF_TOKEN:
        return "EOF_TOKEN";
    case Tokentype::NIL:
        return "NIL";
    case Tokentype::EQUAL:
        return "EQUAL";
    case Tokentype::EQUAL_EQUAL:
        return "EQUAL_EQUAL";
    case Tokentype::BANG_EQUAL:
        return "BANG_EQUAL";
    case Tokentype::LESS:
        return "LESS";
    case Tokentype::LESS_EQUAL:
        return "LESS_EQUAL";
    case Tokentype::GREATER:
        return "GREATER";
    case Tokentype::GREATER_EQUAL:
        return "GREATER_EQUAL";
    case Tokentype::PLUS_EQUAL:
        return "PLUS_EQUAL";
    case Tokentype::MINUS_EQUAL:
        return "MINUS_EQUAL";
    case Tokentype::STAR_EQUAL:
        return "STAR_EQUAL";
    case Tokentype::SLASH_EQUAL:
        return "SLASH_EQUAL";
    case Tokentype::COMMENT:
        return "COMMENT";
    case Tokentype::UNTERMINATED_STRING:
        return "UNTERMINATED_STRING";
    case Tokentype::UNTERMINATED_COMMENT:
        return "UNTERMINATED_COMMENT";
    case Tokentype::UNKNOWN:
        return "UNKNOWN";
    default:
        return "UNDEFINED_TOKEN_TYPE";
    }
}
/* # TODO fix const */
std::optional<ObjFunction*> Compiler::compile()
{
    hadError = false;
    panicMode = false;
    while (!match(Tokentype::EOF_TOKEN)) {
        declaration();
        if (panicMode)
            synchronize();
    }
    return hadError ? std::nullopt : std::make_optional(endCompiler());
}

void Compiler::beginScope()
{
    scope++;
}

void Compiler::endScope()
{
    scope--;
    while (!locals.empty() && locals.back().scopeDepth > scope) {
        locals.pop_back();
    }
}

void Compiler::declareVariable()
{
    if (scope == 0)
        return;

    const Token name = previous;
    for (int i = locals.size() - 1; i > -1; i--) {
        const auto& [token, scopeDepth, isConst, isCaptured] = locals[i];
        if (scopeDepth != -1 && scopeDepth < scope) {
            break;
        }
        if (token.lexeme == name.lexeme) {
            error("Already a variable with this name in this scope.");
        }
    }
    addLocal(name);
}

void Compiler::markInitialized()
{
    if (scope == 0)
        return;
    locals.back().scopeDepth = scope;
}
void Compiler::addLocal(const Token& name)
{
    const bool isConst = previous.type == Tokentype::CONST;
    locals.push_back(Local { name, -1, isConst });
}

int Compiler::resolveLocal(const Token& name, bool currentScopeOnly = false)
{
    for (int i = locals.size() - 1; i >= 0; i--) {
        auto& [token, scopeDepth, isConst, isCaptured] = locals[i];
        if (currentScopeOnly && scopeDepth != scope) {
            break;
        }
        if (name.lexeme == token.lexeme) {
            if (scopeDepth == -1) {
                error("Can't read local variable in its own initializer.");
            }
            return i;
        }
    }
    return -1;
}

void Compiler::block()
{
    while (!check(Tokentype::RIGHTBRACE) && !check(Tokentype::EOF_TOKEN)) {
        declaration();
    }

    consume(Tokentype::RIGHTBRACE, "Expect '}' after block.");
}

Value Compiler::makeString(const std::string& s)
{
    const std::string* internedString = StringInterner::instance().intern(s);
    return { new Obj(ObjString(internedString)) };
}

void Compiler::initRules()
{
    rules[Tokentype::AND] = { nullptr, &Compiler::and_, nullptr, Precedence::AND };
    rules[Tokentype::OR] = { nullptr, &Compiler::or_, nullptr, Precedence::OR };
    rules[Tokentype::LEFTPEREN] = { &Compiler::grouping, &Compiler::call, nullptr, Precedence::CALL };
    rules[Tokentype::MINUS] = { &Compiler::unary, &Compiler::binary, nullptr, Precedence::TERM };
    rules[Tokentype::PLUS] = { nullptr, &Compiler::binary, nullptr, Precedence::TERM };
    rules[Tokentype::SLASH] = { nullptr, &Compiler::binary, nullptr, Precedence::FACTOR };
    rules[Tokentype::STAR] = { nullptr, &Compiler::binary, nullptr, Precedence::FACTOR };
    rules[Tokentype::INTEGER] = { &Compiler::literal, nullptr, nullptr, Precedence::NONE };
    rules[Tokentype::STRING] = { &Compiler::literal, nullptr, nullptr, Precedence::NONE };
    rules[Tokentype::NIL] = { &Compiler::literal, nullptr, nullptr, Precedence::NONE };
    rules[Tokentype::TRUE] = { &Compiler::literal, nullptr, nullptr, Precedence::NONE };
    rules[Tokentype::FALSE] = { &Compiler::literal, nullptr, nullptr, Precedence::NONE };
    rules[Tokentype::BANG] = { &Compiler::unary, nullptr, nullptr, Precedence::NONE };
    rules[Tokentype::EQUAL_EQUAL] = { nullptr, &Compiler::binary, nullptr, Precedence::EQUALITY };
    rules[Tokentype::GREATER] = { nullptr, &Compiler::binary, nullptr, Precedence::COMPARISON };
    rules[Tokentype::GREATER_EQUAL] = { nullptr, &Compiler::binary, nullptr, Precedence::COMPARISON };
    rules[Tokentype::LESS] = { nullptr, &Compiler::binary, nullptr, Precedence::COMPARISON };
    rules[Tokentype::LESS_EQUAL] = { nullptr, &Compiler::binary, nullptr, Precedence::COMPARISON };
    rules[Tokentype::IDENTIFIER] = { &Compiler::variable, nullptr, nullptr, Precedence::NONE };
    rules[Tokentype::INCREMENT] = { &Compiler::prefix, nullptr, &Compiler::postfix, Precedence::CALL };
    rules[Tokentype::DECREMENT] = { &Compiler::prefix, nullptr, &Compiler::postfix, Precedence::CALL };
}

void Compiler::advance()
{
    if (current < tokens.size()) {
        previous = tokens[current++];
    } else {
        errorAtCurrent("Unexpected end of input.");
    }
}

bool Compiler::consume(const Tokentype type, const std::string& message)
{
    if (tokens[current].type == type) {
        advance();
        return true;
    }
    errorAtCurrent(message);
    return false;
}

void Compiler::funDeclaration()
{
    uint8_t global = parseVariable("Expect function name.");
    markInitialized();
    function();
    defineVariable(global);
}

void Compiler::function()
{
    beginScope();
    pushFunction(previous);
    consume(Tokentype::LEFTPEREN, "Expect '(' after function name.");

    int arity = 0;
    if (!check(Tokentype::RIGHTPEREN)) {
        do {
            arity++;
            uint8_t paramConstant = parseVariable("Expect parameter name.");
            defineVariable(paramConstant);
        } while (match(Tokentype::COMMA));
    }
    currentFunction()->arity = arity;
    consume(Tokentype::RIGHTPEREN, "Expect ')' after parameters.");
    consume(Tokentype::LEFTBRACE, "Expect '{' before function body.");
    block();
    endScope();
    emitReturn();
    const auto func = endCompiler();
    emitBytes(cast(OP_CODE::CLOSURE), emitConstant(makeFunction(func)));
    for (int i = 0; i < currentFunction()->upValueCount; i++) {
        emitByte(upvalues[i].isLocal ? 1 : 0);
        emitByte(upvalues[i].index);
    }
}

int Compiler::addUpValue(const uint8_t index, const bool isLocal)
{
    size_t& count = currentFunction()->upValueCount;
    for (size_t i = 0; i < count; i++) {
        const Upvalue& val = upvalues[i];
        if (val.index == index && val.isLocal == isLocal)
            return i;
    }
    upvalues.emplace_back(Upvalue { isLocal, index });
    return count++;
}

int Compiler::resolveUpValue(const Token& name)
{
    if (functions.size() == 1) {
        return -1;
    }
    if (const int local = resolveLocal(name); local != -1) {
        return addUpValue(local, true);
    }
    if (const int upvalue = resolveUpValue(name); upvalue != -1) {
        return addUpValue(upvalue, false);
    }
    return -1;
}

Value Compiler::makeFunction(ObjFunction* function)
{
    function->upValueCount = upvalues.size();
    return { new Obj(ObjFunction(*function)) };
}
ObjFunction* Compiler::currentFunction()
{
    return functions.back();
}

void Compiler::errorAtCurrent(const std::string& message)
{
    errorAt(tokens[current], message);
}

void Compiler::error(const std::string& message)
{
    errorAt(previous, message);
}

void Compiler::errorAt(const Token& token, const std::string& message)
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
    hadError = true;
}

ObjFunction* Compiler::endCompiler()
{
#ifdef DEBUG_PRINT_CODE
    currentChunk().disassembleChunk(currentFunction()->name);
#endif
    auto pFunc = functions.back();
    functions.pop_back();
    scope--;
    return pFunc;
}

void Compiler::expression()
{
    parsePrecedence(Precedence::ASSIGNMENT);
}

void Compiler::parsePrecedence(const Precedence precedence)
{
    advance();
    const ParseFn prefixRule = getRule(previous.type).prefix;
    if (prefixRule == nullptr) {
        error("Expect expression.");
        return;
    }
    const bool canAssign = precedence <= Precedence::ASSIGNMENT;
    (this->*prefixRule)(canAssign);
    while (precedence <= getRule(tokens[current].type).precedence) {
        advance();
        if (const ParseFn infixRule = getRule(previous.type).infix; infixRule != nullptr) {
            (this->*infixRule)(canAssign);
        }
    }
    if (const ParseFn postfixRule = getRule(previous.type).postfix; postfixRule != nullptr) {
        (this->*postfixRule)(false);
    }
    if (canAssign && match(Tokentype::EQUAL)) {
        error("Invalid assignment target.");
    }
}

void Compiler::printStatement()
{
    consume(Tokentype::LEFTPEREN, "Expect '(' after 'print'.");
    expression();
    consume(Tokentype::RIGHTPEREN, "Expect ')' after expression in print statement.");
    emitByte(cast(OP_CODE::PRINT));
}

Compiler::ParseRule Compiler::getRule(const Tokentype type)
{
    const auto it = rules.find(type);
    return it != rules.end() ? it->second : ParseRule { nullptr, nullptr, nullptr, Precedence::NONE };
}

void Compiler::grouping(bool canAssign)
{
    expression();
    consume(Tokentype::RIGHTPEREN, "Expect ')' after expression.");
}

void Compiler::unary(bool canAssign)
{
    const Tokentype& operatorType = previous.type;
    parsePrecedence(Precedence::UNARY);
    switch (operatorType) {
    case Tokentype::MINUS:
        emitByte(cast(OP_CODE::NEG));
        break;
    case Tokentype::BANG:
        emitByte(cast(OP_CODE::NOT));
    default:
        return;
    }
}
void Compiler::switchStatement()
{
    expression();
    consume(Tokentype::LEFTBRACE, "Expect '{' after switch condition.");
    std::vector<int> endJumps;
    std::vector<int> nextCaseJumps;
    bool hasDefault = false;
    int defaultJump = -1;
    while (!check(Tokentype::RIGHTBRACE) && !check(Tokentype::EOF_TOKEN)) {
        if (match(Tokentype::IDENTIFIER) && previous.lexeme == "_") {
            if (hasDefault) {
                error("Cannot have multiple default cases in switch statement.");
            }
            hasDefault = true;
            consume(Tokentype::ARROW, "Expect '->' after default case.");
            defaultJump = current;
            for (int jump : nextCaseJumps) {
                patchJump(jump);
            }
            nextCaseJumps.clear();
            statement();
            endJumps.push_back(emitJump(cast(OP_CODE::JUMP)));
        } else {
            for (int jump : nextCaseJumps) {
                patchJump(jump);
            }
            nextCaseJumps.clear();
            emitByte(cast(OP_CODE::DUP));
            expression();
            consume(Tokentype::ARROW, "Expect '->' after case value.");
            emitByte(cast(OP_CODE::EQUAL));
            int jumpToNextCase = emitJump(cast(OP_CODE::JUMP_IF_FALSE));
            nextCaseJumps.push_back(jumpToNextCase);
            emitByte(cast(OP_CODE::POP));
            emitByte(cast(OP_CODE::POP));
            statement();
            endJumps.push_back(emitJump(cast(OP_CODE::JUMP)));
        }
    }
    for (int jump : nextCaseJumps) {
        patchJump(jump);
    }
    if (hasDefault) {
        patchJump(defaultJump);
    } else {
        emitByte(cast(OP_CODE::POP));
    }
    for (int jump : endJumps) {
        patchJump(jump);
    }
    consume(Tokentype::RIGHTBRACE, "Expect '}' at end of switch statement.");
}

void Compiler::prefix(bool canAssign)
{
    const Tokentype operatorType = previous.type;
    parsePrecedence(Precedence::UNARY);
    emitConstant(operatorType == Tokentype::INCREMENT ? Value(1.0) : Value(-1.0));
    emitByte(cast(OP_CODE::ADD));
}

void Compiler::postfix(bool canAssign)
{
    const Token name = tokens[current - 2];
    int arg = resolveLocal(name);
    uint8_t setOp;
    if (arg != -1) {
        setOp = cast(OP_CODE::SET_LOCAL);
    } else {
        arg = identifierConstant(name);
        setOp = cast(OP_CODE::SET_GLOBAL);
    }

    emitByte(cast(OP_CODE::DUP));
    switch (previous.type) {
    case Tokentype::INCREMENT:
        emitConstant(Value(1.0));
        emitByte(cast(OP_CODE::ADD));
        break;
    case Tokentype::DECREMENT:
        emitConstant(Value(-1.0));
        emitByte(cast(OP_CODE::ADD));
        break;
    default:
        error("Unexpected postfix operator.");
        return;
    }
    emitBytes(setOp, static_cast<uint8_t>(arg));
    emitByte(cast(OP_CODE::POP));
}

void Compiler::binary(bool canAssign)
{
    const Tokentype operatorType = previous.type;
    const auto& [prefix, infix, postfix, precedence] = getRule(operatorType);
    parsePrecedence(static_cast<Precedence>(static_cast<int>(precedence) + 1));
    switch (operatorType) {
    case Tokentype::PLUS:
        emitByte(cast(OP_CODE::ADD));
        break;
    case Tokentype::MINUS:
        emitBytes(cast(OP_CODE::NEG), cast(OP_CODE::ADD));
        break;
    case Tokentype::STAR:
        emitByte(cast(OP_CODE::MULT));
        break;
    case Tokentype::SLASH:
        emitByte(cast(OP_CODE::DIV));
        break;
    case Tokentype::BANG_EQUAL:
        emitBytes(cast(OP_CODE::EQUAL), cast(OP_CODE::NOT));
        break;
    case Tokentype::EQUAL_EQUAL:
        emitByte(cast(OP_CODE::EQUAL));
        break;
    case Tokentype::GREATER:
        emitByte(cast(OP_CODE::GREATER));
        break;
    case Tokentype::GREATER_EQUAL:
        emitBytes(cast(OP_CODE::LESS), cast(OP_CODE::NOT));
        break;
    case Tokentype::LESS:
        emitByte(cast(OP_CODE::LESS));
        break;
    case Tokentype::LESS_EQUAL:
        emitBytes(cast(OP_CODE::GREATER), cast(OP_CODE::NOT));
        break;
    default:
        std::cout << "Unhandled binary operator: " << tokenTypeToString(operatorType) << std::endl;
        return;
    }
}

void Compiler::literal(bool canAssign)
{
    switch (previous.type) {
    case Tokentype::TRUE:
        emitByte(cast(OP_CODE::TRUE));
        break;
    case Tokentype::FALSE:
        emitByte(cast(OP_CODE::FALSE));
        break;
    case Tokentype::INTEGER: {
        const double value = std::stod(previous.lexeme);
        emitConstant(Value(value));
    } break;
    case Tokentype::STRING: {
        const std::string value = previous.lexeme.substr(1, previous.lexeme.length() - 2);
        emitConstant(makeString(value));
    } break;
    case Tokentype::NIL:
        emitByte(cast(OP_CODE::NIL));
        break;
    default:
        return;
    }
}

void Compiler::emitReturn()
{
    emitBytes(cast(OP_CODE::NIL), cast(OP_CODE::RETURN));
}

uint8_t Compiler::emitConstant(const Value& value)
{
    return currentChunk().writeConstant(value, previous.line);
}

bool Compiler::check(const Tokentype type) const
{
    return tokens[current].type == type;
}

int Compiler::emitJump(const uint8_t instruction)
{
    emitByte(instruction);
    emitByte(0xff);
    emitByte(0xff);
    return currentChunk().code.size() - 2;
}

void Compiler::patchJump(const int offset)
{
    const int jump = currentChunk().code.size() - offset - 2;

    if (jump > UINT16_MAX) {
        error("Too much code to jump over.");
    }
    currentChunk().code[offset] = (jump >> 8) & 0xff;
    currentChunk().code[offset + 1] = jump & 0xff;
}

void Compiler::ifStatement()
{
    consume(Tokentype::LEFTPEREN, "Expect '(' after 'if'.");
    expression();
    consume(Tokentype::RIGHTPEREN, "Expect ')' after condition.");

    const int thenJump = emitJump(cast(OP_CODE::JUMP_IF_FALSE));
    emitByte(cast(OP_CODE::POP));
    statement();

    const int elseJump = emitJump(cast(OP_CODE::JUMP));

    patchJump(thenJump);
    emitByte(cast(OP_CODE::POP));

    if (match(Tokentype::ELSE)) {
        statement();
    }

    patchJump(elseJump);
}

void Compiler::expressionStatement()
{
    expression();
    consume(Tokentype::SEMICOLON, "Expect ';' after expression");
    if (panicMode) {
        synchronize();
    }
}

void Compiler::emitLoop(const int loopStart)
{
    emitByte(cast(OP_CODE::LOOP));
    const int offset = currentChunk().code.size() - loopStart + 2;
    if (offset > UINT16_MAX)
        error("Loop body too large.");

    emitByte((offset >> 8) & 0xff);
    emitByte(offset & 0xff);
}

void Compiler::whileStatement()
{
    LoopInfo loop;
    loop.start = currentChunk().code.size();
    loop.scopeDepth = scope;
    loop.continueTarget = loop.start;
    loopStack.push_back(loop);

    consume(Tokentype::LEFTPEREN, "Expect '(' after 'while'.");
    expression();
    consume(Tokentype::RIGHTPEREN, "Expect ')' after condition.");

    int exitJump = emitJump(cast(OP_CODE::JUMP_IF_FALSE));
    emitByte(cast(OP_CODE::POP));
    statement();
    emitLoop(loop.start);

    patchJump(exitJump);
    emitByte(cast(OP_CODE::POP));
    for (int offset : loopStack.back().breaks) {
        patchJump(offset);
    }

    loopStack.pop_back();
}
void Compiler::forStatement()
{
    beginScope();
    consume(Tokentype::LEFTPEREN, "Expect '(' after 'for'.");
    if (match(Tokentype::SEMICOLON)) {
    } else if (match(Tokentype::LET)) {
        variableDeclaration();
    } else {
        expressionStatement();
    }
    LoopInfo loop;
    loop.start = currentChunk().code.size();
    loop.scopeDepth = scope;
    int exitJump = -1;
    if (!match(Tokentype::SEMICOLON)) {
        expression();
        consume(Tokentype::SEMICOLON, "Expect ';' after loop condition.");
        exitJump = emitJump(cast(OP_CODE::JUMP_IF_FALSE));
        emitByte(cast(OP_CODE::POP));
    }
    if (!match(Tokentype::RIGHTPEREN)) {
        int bodyJump = emitJump(cast(OP_CODE::JUMP));
        int incrementStart = currentChunk().code.size();
        expression();
        emitByte(cast(OP_CODE::POP));
        consume(Tokentype::RIGHTPEREN, "Expect ')' after for clauses.");

        emitLoop(loop.start);
        loop.start = incrementStart;
        patchJump(bodyJump);
    }
    loop.continueTarget = loop.start;
    loopStack.push_back(loop);
    statement();
    emitLoop(loop.start);
    if (exitJump != -1) {
        patchJump(exitJump);
        emitByte(cast(OP_CODE::POP));
    }
    for (int offset : loopStack.back().breaks) {
        patchJump(offset);
    }
    loopStack.pop_back();
    endScope();
}
void Compiler::call(bool canAssign)
{
    uint8_t argCount = argumentList();
    emitBytes(cast(OP_CODE::CALL), argCount);
}

uint8_t Compiler::argumentList()
{
    uint8_t argCount = 0;
    if (!check(Tokentype::RIGHTPEREN)) {
        do {
            expression();
            argCount++;
        } while (match(Tokentype::COMMA));
    }
    consume(Tokentype::RIGHTPEREN, "Expect ')' after arguments.");
    return argCount;
}
void Compiler::returnStatement()
{
    if (currentFunction()->name == "Main") {
        error("Can't return from top-level code.");
    }

    if (match(Tokentype::SEMICOLON)) {
        emitReturn();
    } else {
        expression();
        consume(Tokentype::SEMICOLON, "Expect ';' after return value.");
        emitByte(cast(OP_CODE::RETURN));
    }
}

void Compiler::statement()
{
    if (match(Tokentype::PRINT)) {
        printStatement();
    } else if (match(Tokentype::RETURN)) {
        returnStatement();
    } else if (match(Tokentype::SWITCH)) {
        switchStatement();
    } else if (match(Tokentype::FOR)) {
        forStatement();
    } else if (match(Tokentype::BREAK)) {
        breakStatement();
    } else if (match(Tokentype::CONTINUE)) {
        continueStatement();
    } else if (match(Tokentype::LEFTBRACE)) {
        beginScope();
        block();
        endScope();
    } else if (match(Tokentype::IF)) {
        ifStatement();
    } else if (match(Tokentype::WHILE)) {
        whileStatement();
    } else {
        expressionStatement();
    }
}

uint8_t Compiler::identifierConstant(const Token& token)
{
    const auto constant = makeString(token.lexeme);
    Value indexValue;
    const auto it = stringConstants.find(constant.to_string());
    if (it == stringConstants.end()) {
        stringConstants[token.lexeme] = emitConstant(constant);
        return stringConstants[token.lexeme];
    }
    return it->second;
}

uint8_t Compiler::parseVariable(const std::string& errorMessage)
{
    consume(Tokentype::IDENTIFIER, errorMessage);

    declareVariable();
    if (scope > 0)
        return 0;

    return identifierConstant(previous);
}

void Compiler::defineVariable(const uint8_t global)
{
    if (scope > 0) {
        markInitialized();
        return;
    }
    emitBytes(cast(OP_CODE::DEFINE_GLOBAL), global);
}

void Compiler::emitByte(const uint8_t byte)
{
    currentChunk().writeChunk(byte, previous.line);
}

void Compiler::emitBytes(const uint8_t byte1, const uint8_t byte2)
{
    emitByte(byte1);
    emitByte(byte2);
}

void Compiler::variableDeclaration()
{
    bool isConst = previous.type == Tokentype::CONST;
    uint8_t global = parseVariable("Expect variable name.");

    if (match(Tokentype::EQUAL)) {
        expression();
    } else {
        if (isConst) {
            error("Const variables must be initialized.");
            return;
        }
        emitByte(cast(OP_CODE::NIL));
    }

    consume(Tokentype::SEMICOLON, "Expect ';' after variable declaration");

    if (scope > 0) {
        markInitialized();
    } else {
        if (isConst) {
            constGlobals.insert(previous.lexeme);
        }
        defineVariable(global);
    }
}

void Compiler::variable(const bool canAssign)
{
    namedVariable(previous, canAssign);
}

void Compiler::declaration()
{
    if (panicMode)
        synchronize();
    if (match(Tokentype::FUN)) {
        funDeclaration();
    } else if (match(Tokentype::LET) || match(Tokentype::CONST)) {
        variableDeclaration();
    } else {
        statement();
    }
}

bool Compiler::match(const Tokentype& type)
{
    if (!check(type))
        return false;
    advance();
    return true;
}
void Compiler::and_(bool canAssign)
{
    const int endJump = emitJump(cast(OP_CODE::JUMP_IF_FALSE));
    emitByte(cast(OP_CODE::POP));
    parsePrecedence(Precedence::AND);
    patchJump(endJump);
}

void Compiler::or_(bool canAssign)
{
    const int elseJump = emitJump(cast(OP_CODE::JUMP_IF_FALSE));
    const int endJump = emitJump(cast(OP_CODE::JUMP));
    patchJump(elseJump);
    emitByte(cast(OP_CODE::POP));
    parsePrecedence(Precedence::OR);
    patchJump(endJump);
}
void Compiler::synchronize()
{
    panicMode = false;
    while (current < tokens.size() && tokens[current].type != Tokentype::EOF_TOKEN) {
        if (previous.type == Tokentype::SEMICOLON)
            return;
        switch (tokens[current].type) {
        case Tokentype::CLASS:
        case Tokentype::FUN:
        case Tokentype::LET:
        case Tokentype::CONST:
        case Tokentype::FOR:
        case Tokentype::IF:
        case Tokentype::WHILE:
        case Tokentype::PRINT:
        case Tokentype::RETURN:
            return;
        default:
            break;
        }
        advance();
    }
}

void Compiler::namedVariable(const Token& name, const bool canAssign)
{
    uint8_t getOp, setOp;
    int arg = resolveLocal(name);
    if (arg != -1) {
        getOp = cast(OP_CODE::GET_LOCAL);
        setOp = cast(OP_CODE::SET_LOCAL);
    } else if ((arg = resolveUpValue(name)) != -1) {
        getOp = cast(OP_CODE::GET_UPVALUE);
        setOp = cast(OP_CODE::SET_UPVALUE);
    } else {
        arg = identifierConstant(name);
        getOp = cast(OP_CODE::GET_GLOBAL);
        setOp = cast(OP_CODE::SET_GLOBAL);
    }
    if (canAssign && match(Tokentype::EQUAL)) {
        expression();
        emitBytes(setOp, static_cast<uint8_t>(arg));
    } else {
        emitBytes(getOp, static_cast<uint8_t>(arg));
    }
}

void Compiler::breakStatement()
{
    if (loopStack.empty()) {
        error("Cannot use 'break' outside of a loop.");
        return;
    }
    consume(Tokentype::SEMICOLON, "Expect ';' after 'break'.");
    int jump = emitJump(cast(OP_CODE::JUMP));
    loopStack.back().breaks.push_back(jump);
}

void Compiler::continueStatement()
{
    if (loopStack.empty()) {
        error("Cannot use 'continue' outside of a loop.");
        return;
    }
    consume(Tokentype::SEMICOLON, "Expect ';' after 'continue'.");
    emitLoop(loopStack.back().continueTarget);
}
