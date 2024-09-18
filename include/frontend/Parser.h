#pragma once
#include "Expression.h"
#include "Statement.h"
#include <memory>
#include <unordered_map>
#include <vector>

enum class Precedence {
    NONE,
    ASSIGNMENT, // =
    OR, // or
    AND, // and
    EQUALITY, // == !=
    COMPARISON, // < > <= >=
    TERM, // + -
    FACTOR, // * /
    UNARY, // ! -
    CALL, // . ()
    PRIMARY
};

class Parser {
public:
    Parser(std::vector<Token>& tokens)
        : tokens(tokens)
        , current(0)
        , hadError(false)
        , panicMode(false)
    {
        initRules();
    }

    std::vector<std::unique_ptr<Statement>> parseProgram();

private:
    std::vector<Token> tokens;
    size_t current;
    bool hadError;
    bool panicMode;

    bool match(Tokentype type);
    using ParseFn = std::unique_ptr<Expression> (Parser::*)(bool canAssign);
    using InfixFn = std::unique_ptr<Expression> (Parser::*)(std::unique_ptr<Expression> left, bool canAssign);
    struct ParseRule {
        ParseFn prefix;
        InfixFn infix;
        InfixFn postfix;
        Precedence precedence;
    };
    std::unordered_map<Tokentype, ParseRule> rules;

    void initRules();
    void advance();

    bool consume(Tokentype type, const std::string& message);
    void errorAtCurrent(const std::string& message);
    void error(const std::string& message);
    void errorAt(const Token& token, const std::string& message);
    std::unique_ptr<Expression> parsePrecedence(Precedence precedence);

    std::unique_ptr<Expression> and_(std::unique_ptr<Expression> left, bool canAssign);
    bool check(Tokentype type) const;
    Token peek() const;
    Token previous;
    Token previousToken();
    Token previousToken(int by);
    void synchronize();
    std::unique_ptr<Expression> expression();
    std::unique_ptr<Statement> statement();
    std::unique_ptr<Statement> declaration();
    std::unique_ptr<Statement> printStatement();
    std::unique_ptr<Statement> expressionStatement();
    std::unique_ptr<Statement> ifStatement();
    std::unique_ptr<Statement> whileStatement();
    std::unique_ptr<Statement> forStatement();
    std::unique_ptr<Statement> returnStatement();
    std::unique_ptr<Expression> grouping(bool canAssign);
    std::unique_ptr<Expression> unary(bool canAssign);
    std::unique_ptr<Expression> binary(std::unique_ptr<Expression> left, bool canAssign);
    std::unique_ptr<Expression> literal(bool canAssign);
    std::unique_ptr<Expression> variable(bool canAssign);
    std::unique_ptr<Expression> call(std::unique_ptr<Expression> callee, bool canAssign);
    std::unique_ptr<Expression> or_(std::unique_ptr<Expression> left, bool canAssign);
    std::unique_ptr<Expression> prefix(bool canAssign);
    std::unique_ptr<Expression> postfix(std::unique_ptr<Expression> left, bool canAssign);
    std::unique_ptr<Statement> blockStatement();
    std::unique_ptr<Statement> functionDeclaration(const std::string& kind);
    std::unique_ptr<Statement> variableDeclaration();
    ParseRule getRule(Tokentype type);
};
