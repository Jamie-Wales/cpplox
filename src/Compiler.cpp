#include "Compiler.h"
#include "Instructions.h"
#include "Stringinterner.h"
#include "Token.h"
#include "Value.h"
#include <optional>

std::optional<Chunk> Compiler::compile()
{
    hadError = false;
    panicMode = false;
    while (!match(Tokentype::EOF_TOKEN)) {
        declaration();
    }
    endCompiler();
    return hadError ? std::nullopt : std::make_optional(currentChunk);
}

Value Compiler::makeString(const std::string& s)
{
    const std::string* internedString = StringInterner::instance().intern(s);
    return Value(new Obj(ObjString(internedString)));
}

void Compiler::initRules()
{
    rules[Tokentype::LEFTPEREN] = { &Compiler::grouping, nullptr, Precedence::NONE };
    rules[Tokentype::MINUS] = { &Compiler::unary, &Compiler::binary, Precedence::TERM };
    rules[Tokentype::RIGHTPEREN] = { nullptr, nullptr, Precedence::NONE };
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
    rules[Tokentype::IDENTIFIER] = { nullptr, nullptr, Precedence::NONE };
}

void Compiler::advance()
{
    if (current < tokens.size()) {
        previous = tokens[current++];
    } else {
        errorAtCurrent("Unexpected end of input.");
    }
}

void Compiler::consume(Tokentype type, const std::string& message)
{
    if (tokens[current].type == type) {
        advance();
    } else {
        errorAtCurrent(message);
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

void Compiler::parsePrecedence(Precedence precedence)
{
    ParseFn prefixRule = getRule(tokens[current].type).prefix;
    if (prefixRule == nullptr) {
        error("Expect expression.");
        return;
    }
    advance();
    (this->*prefixRule)();
    while (precedence <= getRule(tokens[current].type).precedence) {
        advance();
        ParseFn infixRule = getRule(previous.type).infix;
        (this->*infixRule)();
    }
}

Compiler::ParseRule Compiler::getRule(Tokentype type)
{
    auto it = rules.find(type);
    return it != rules.end() ? it->second : ParseRule { nullptr, nullptr, Precedence::NONE };
}

void Compiler::grouping()
{
    expression();
    consume(Tokentype::RIGHTPEREN, "Expect ')' after expression.");
}

void Compiler::unary()
{
    Tokentype operatorType = previous.type;
    parsePrecedence(Precedence::UNARY);
    switch (operatorType) {
    case Tokentype::MINUS:
        emitByte(OP_CODE::NEG);
        break;
    case Tokentype::BANG:
        emitByte(OP_CODE::NOT);
    default:
        return;
    }
}

void Compiler::binary()
{
    Tokentype operatorType = previous.type;
    ParseRule rule = getRule(operatorType);
    parsePrecedence(static_cast<Precedence>(static_cast<int>(rule.precedence) + 1));
    switch (operatorType) {
    case Tokentype::PLUS:
        emitByte(OP_CODE::ADD);
        break;
    case Tokentype::MINUS:
        emitBytes(OP_CODE::NEG, OP_CODE::ADD);
        break;
    case Tokentype::STAR:
        emitByte(OP_CODE::MULT);
        break;
    case Tokentype::SLASH:
        emitByte(OP_CODE::DIV);
        break;
    case Tokentype::BANG_EQUAL:
        emitBytes(OP_CODE::EQUAL, OP_CODE::NOT);
        break;
    case Tokentype::EQUAL_EQUAL:
        emitByte(OP_CODE::EQUAL);
        break;
    case Tokentype::GREATER:
        emitByte(OP_CODE::GREATER);
        break;
    case Tokentype::GREATER_EQUAL:
        emitBytes(OP_CODE::LESS, OP_CODE::NOT);
        break;
    case Tokentype::LESS:
        emitByte(OP_CODE::LESS);
        break;
    case Tokentype::LESS_EQUAL:
        emitBytes(OP_CODE::GREATER, OP_CODE::NOT);
        break;
    default:
        return;
    }
}

void Compiler::literal()
{
    switch (previous.type) {
    case Tokentype::TRUE:
        emitByte(OP_CODE::TRUE);
        break;
    case Tokentype::FALSE:
        emitByte(OP_CODE::FALSE);
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
        emitByte(OP_CODE::NIL);
        break;
    default:
        return;
    }
}
void Compiler::emitByte(uint8_t byte)
{
    currentChunk.writeChunk(byte, previous.line);
}

void Compiler::emitBytes(uint8_t byte1, uint8_t byte2)
{
    emitByte(byte1);
    emitByte(byte2);
}

void Compiler::emitReturn()
{
    emitByte(OP_CODE::RETURN);
}

uint8_t Compiler::emitConstant(const Value& value)
{
    return currentChunk.writeConstant(value, previous.line);
}
