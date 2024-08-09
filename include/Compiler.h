#pragma once
#include "Chunk.h"
#include "Instructions.h"
#include "Token.h"
#include <iostream>
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

class Compiler {
public:
    Compiler(const std::vector<Token>& tokens)
        : tokens(tokens)
        , current(0)
    {
        initRules();
    }

    std::optional<Chunk> compile()
    {
        hadError = false;
        panicMode = false;

        expression();
        consume(Tokentype::EOF_TOKEN, "Expect end of expression.");
        endCompiler();

        return hadError ? std::nullopt : std::make_optional(currentChunk);
    }

private:
    using ParseFn = void (Compiler::*)();

    struct ParseRule {
        ParseFn prefix;
        ParseFn infix;
        Precedence precedence;
    };

    Chunk currentChunk { 100 };
    const std::vector<Token>& tokens;
    bool hadError = false;
    bool panicMode = false;
    size_t current;
    Token previous;
    std::unordered_map<Tokentype, ParseRule> rules;

    void initRules()
    {
        rules[Tokentype::LEFTPEREN] = { &Compiler::grouping, nullptr, Precedence::NONE };
        rules[Tokentype::MINUS] = { &Compiler::unary, &Compiler::binary, Precedence::TERM };
        rules[Tokentype::RIGHTPEREN] = { nullptr, nullptr, Precedence::NONE };
        rules[Tokentype::PLUS] = { nullptr, &Compiler::binary, Precedence::TERM };
        rules[Tokentype::SLASH] = { nullptr, &Compiler::binary, Precedence::FACTOR };
        rules[Tokentype::STAR] = { nullptr, &Compiler::binary, Precedence::FACTOR };
        rules[Tokentype::INTEGER] = { &Compiler::number, nullptr, Precedence::NONE };
    }

    void advance()
    {
        if (current < tokens.size()) {
            previous = tokens[current++];
        } else {
            errorAtCurrent("Unexpected end of input.");
        }
    }

    void consume(Tokentype type, const std::string& message)
    {
        if (tokens[current].type == type) {
            advance();
        } else {
            errorAtCurrent(message);
        }
    }

    void errorAtCurrent(const std::string& message)
    {
        errorAt(tokens[current], message);
    }

    void error(const std::string& message)
    {
        errorAt(previous, message);
    }

    void errorAt(const Token& token, const std::string& message)
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

    void endCompiler()
    {
        emitReturn();
    }

    void expression()
    {
        parsePrecedence(Precedence::ASSIGNMENT);
    }

    void parsePrecedence(Precedence precedence)
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
    ParseRule getRule(Tokentype type)
    {
        auto it = rules.find(type);
        return it != rules.end() ? it->second : ParseRule { nullptr, nullptr, Precedence::NONE };
    }

    void grouping()
    {
        expression();
        consume(Tokentype::RIGHTPEREN, "Expect ')' after expression.");
    }

    void unary()
    {
        Tokentype operatorType = previous.type;

        parsePrecedence(Precedence::UNARY);

        switch (operatorType) {
        case Tokentype::MINUS:
            emitByte(OP_CODE::NEG);
            break;
        default:
            return;
        }
    }

    void binary()
    {
        Tokentype operatorType = previous.type;
        ParseRule rule = getRule(operatorType);
        parsePrecedence(static_cast<Precedence>(static_cast<int>(rule.precedence) + 1));

        switch (operatorType) {
        case Tokentype::PLUS:
            emitByte(OP_CODE::ADD);
            break;
        case Tokentype::MINUS:
            emitByte(OP_CODE::SUB);
            break;
        case Tokentype::STAR:
            emitByte(OP_CODE::MULT);
            break;
        case Tokentype::SLASH:
            emitByte(OP_CODE::DIV);
            break;
        default:
            return;
        }
    }

    void number()
    {
        double value = std::stod(previous.lexeme);
        emitConstant(value);
    }

    void emitByte(uint8_t byte)
    {
        currentChunk.writeChunk(byte, previous.line);
    }

    void emitBytes(uint8_t byte1, uint8_t byte2)
    {
        emitByte(byte1);
        emitByte(byte2);
    }

    void emitReturn()
    {
        emitByte(OP_CODE::RETURN);
    }

    void emitConstant(double value)
    {
        currentChunk.writeConstant(value, previous.line);
    }
};
