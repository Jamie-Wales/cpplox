#include "Compiler.h"
#include "Instructions.h"
#include "Stringinterner.h"
#include "Token.h"
#include "Value.h"
#include <cstdint>
#include <iostream>
#include <optional>

std::string tokenTypeToString(Tokentype type)
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

std::optional<Chunk> Compiler::compile()
{
    hadError = false;
    panicMode = false;
    while (!match(Tokentype::EOF_TOKEN)) {
        declaration();
        if (panicMode) synchronize();
    }
    endCompiler();
    return hadError ? std::nullopt : std::make_optional(currentChunk);
}


void Compiler::beginScope()
{
    scope++;
}

void Compiler::endScope()
{
    scope--;
    while (!locals.empty() && locals.back().scopeDepth > scope) {
        emitByte(cast(OP_CODE::POP));
        locals.pop_back();
    }
}

void Compiler::declareVariable()
{
    if (scope == 0)
        return;

    Token& name = previous;
    for (size_t i = locals.size() - 1; i > -1; i--) {
        Local* local = &locals[i];
        if (local->scopeDepth != -1 && local->scopeDepth < scope) {
            break;
        }
        if (local->token.lexeme == name.lexeme) {
            error("Already a variable with this name in this scope.");
        }
    }
    addLocal(name);
}

void Compiler::addLocal(Token& name)
{
    bool isConst = previous.type == Tokentype::CONST;
    locals.emplace_back(Local { name, -1, isConst });
}

void Compiler::markInitialized()
{
    if (scope == 0)
        return;
    locals.back().scopeDepth = scope;
}

int Compiler::resolveLocal(const Token& name)
{
    for (int i = locals.size() - 1; i > -1; i--) {
        Local local = locals[i];
        if (name.lexeme == local.token.lexeme) {
            if (local.scopeDepth == -1) {
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
    rules[Tokentype::AND] = { nullptr, &Compiler::binary, Precedence::AND },
    rules[Tokentype::OR] = { nullptr, &Compiler::binary, Precedence::OR },
    rules[Tokentype::LEFTPEREN] = { &Compiler::grouping, nullptr, Precedence::NONE };
    rules[Tokentype::MINUS] = { &Compiler::unary, &Compiler::binary, Precedence::TERM };
    rules[Tokentype::PLUS] = { nullptr, &Compiler::binary, Precedence::TERM };
    rules[Tokentype::SLASH] = { nullptr, &Compiler::binary, Precedence::FACTOR };
    rules[Tokentype::STAR] = { nullptr, &Compiler::binary, Precedence::FACTOR };
    rules[Tokentype::INTEGER] = { &Compiler::literal, nullptr, Precedence::NONE };
    rules[Tokentype::STRING] = { &Compiler::literal, nullptr, Precedence::NONE };
    rules[Tokentype::NIL] = { &Compiler::literal, nullptr, Precedence::NONE };
    rules[Tokentype::TRUE] = { &Compiler::literal, nullptr, Precedence::NONE };
    rules[Tokentype::FALSE] = { &Compiler::literal, nullptr, Precedence::NONE };
    rules[Tokentype::BANG] = { &Compiler::unary, nullptr, Precedence::NONE };
    rules[Tokentype::EQUAL_EQUAL] = { nullptr, &Compiler::binary, Precedence::EQUALITY };
    rules[Tokentype::GREATER] = { nullptr, &Compiler::binary, Precedence::COMPARISON };
    rules[Tokentype::GREATER_EQUAL] = { nullptr, &Compiler::binary, Precedence::COMPARISON };
    rules[Tokentype::LESS] = { nullptr, &Compiler::binary, Precedence::COMPARISON };
    rules[Tokentype::LESS_EQUAL] = { nullptr, &Compiler::binary, Precedence::COMPARISON };
    rules[Tokentype::IDENTIFIER] = { &Compiler::variable, nullptr, Precedence::NONE };
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
    } else {
        errorAtCurrent(message);
        return false;
    }
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

void Compiler::endCompiler()
{
    emitReturn();
}

void Compiler::expression()
{
    parsePrecedence(Precedence::ASSIGNMENT);
}

void Compiler::parsePrecedence(const Precedence precedence)
{
    const bool canAssign = precedence <= Precedence::ASSIGNMENT;
    const ParseFn prefixRule = getRule(tokens[current].type).prefix;
    if (prefixRule == nullptr) {
        error("Expect expression.");
        return;
    }

    advance();
    (this->*prefixRule)(canAssign);

    while (precedence <= getRule(tokens[current].type).precedence) {
        advance();
        const ParseFn infixRule = getRule(previous.type).infix;
        (this->*infixRule)(canAssign);
    }
}

Compiler::ParseRule Compiler::getRule(const Tokentype type)
{
    const auto it = rules.find(type);
    return it != rules.end() ? it->second : ParseRule { nullptr, nullptr, Precedence::NONE };
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

void Compiler::binary(bool canAssign)
{
    const Tokentype operatorType = previous.type;
    const ParseRule& rule = getRule(operatorType);
    parsePrecedence(static_cast<Precedence>(static_cast<int>(rule.precedence) + 1));
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
        case Tokentype::AND:
            emitByte(cast(OP_CODE::AND));
            break;
        case Tokentype::OR:
            emitByte(cast(OP_CODE::OR));
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
            double value = std::stod(previous.lexeme);
            emitConstant(Value(value));
        } break;
        case Tokentype::STRING: {
            std::string value = previous.lexeme.substr(1, previous.lexeme.length() - 2);
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
    emitByte(cast(OP_CODE::RETURN));
}

uint8_t Compiler::emitConstant(const Value& value)
{
    return currentChunk.writeConstant(value, previous.line);
}

bool Compiler::check(const Tokentype type) const
{
    return tokens[current].type == type;
}

void Compiler::printStatement()
{
    consume(Tokentype::LEFTPEREN, "Expect '(' after 'print'.");
    expression();
    consume(Tokentype::RIGHTPEREN, "Expect ')' after expression in print statement.");
    consume(Tokentype::SEMICOLON, "Expect ';' after print statement.");
    emitByte(cast(OP_CODE::PRINT));
}

void Compiler::expressionStatement()
{
    expression();
    if (!panicMode) {
        consume(Tokentype::SEMICOLON, "Expect ';' after expression.");
    } else {
        synchronize();
    }
    emitByte(cast(OP_CODE::POP));
}
void Compiler::statement()
{
    if (match(Tokentype::PRINT)) {
        printStatement();
    } else if (match(Tokentype::LEFTBRACE)) {
        beginScope();
        block();
        endScope();
    } else {
        expressionStatement();
    }
}

uint8_t Compiler::identifierConstant(Token& token)
{
    auto constant = makeString(token.lexeme);
    Value indexValue;
    auto it = stringConstants.find(constant.to_string());
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

void Compiler::defineVariable(uint8_t global)
{
    if (scope > 0) {
        markInitialized();
        return;
    }
    emitBytes(cast(OP_CODE::DEFINE_GLOBAL), global);
}

void Compiler::emitByte(const uint8_t byte)
{
    currentChunk.writeChunk(byte, previous.line);
}

void Compiler::emitBytes(const uint8_t byte1, const uint8_t byte2)
{
    emitByte(byte1);
    emitByte(byte2);
}

void Compiler::variableDeclaration()
{
    bool isConst = previous.type == Tokentype::CONST;
    const uint8_t global = parseVariable("Expect variable name.");
    Token name = previous;
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
        locals.back().isConst = isConst;
    } else {
        if (isConst) {
            constGlobals.insert(name.lexeme); // Use the stored variable name
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
    if (match(Tokentype::LET) || match(Tokentype::CONST)) {
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

void Compiler::synchronize()
{
    panicMode = false;

    while (current < tokens.size() && tokens[current].type != Tokentype::EOF_TOKEN) {
        if (previous.type == Tokentype::SEMICOLON) return;

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
                // Do nothing
                break;
        }

        advance();
    }
}
void Compiler::namedVariable(Token& name, bool canAssign)
{
    uint8_t getOp, setOp;
    int arg = resolveLocal(name);

    if (arg != -1) {
        getOp = cast(OP_CODE::GET_LOCAL);
        setOp = cast(OP_CODE::SET_LOCAL);
        if (canAssign && check(Tokentype::EQUAL)) {
            if (locals[arg].isConst) {
                error("Cannot reassign to const local variable.");
                // Don't return, continue processing
            }
            advance();
            expression();
            emitBytes(setOp, static_cast<uint8_t>(arg));
        } else {
            emitBytes(getOp, static_cast<uint8_t>(arg));
        }
    } else {
        arg = identifierConstant(name);
        getOp = cast(OP_CODE::GET_GLOBAL);
        setOp = cast(OP_CODE::SET_GLOBAL);
        if (canAssign && check(Tokentype::EQUAL)) {
            if (constGlobals.find(name.lexeme) != constGlobals.end()) {
                error("Cannot reassign to const global variable.");
                // Don't return, continue processing
            }
            advance();
            expression();
            emitBytes(setOp, static_cast<uint8_t>(arg));
        } else {
            emitBytes(getOp, static_cast<uint8_t>(arg));
        }
    }
}
