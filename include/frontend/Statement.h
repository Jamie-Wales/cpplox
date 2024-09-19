#pragma once
#include "Expression.h"
#include <memory>
#include <variant>
#include <vector>

class Statement;

class ExpressionStatement {
public:
    std::unique_ptr<Expression> expression;
    int line;

    ExpressionStatement(std::unique_ptr<Expression> expression, int line)
        : expression(std::move(expression))
        , line(line)
    {
    }
};

class PrintStatement {
public:
    std::unique_ptr<Expression> expression;
    int line;

    PrintStatement(std::unique_ptr<Expression> expression, int line)
        : expression(std::move(expression))
        , line(line)
    {
    }
};

class VariableDeclaration {
public:
    Token name;
    std::unique_ptr<Expression> initializer;
    bool isConst;
    int line;

    VariableDeclaration(Token name, std::unique_ptr<Expression> initializer, bool isConst, int line)
        : name(std::move(name))
        , initializer(std::move(initializer))
        , isConst(isConst)
        , line(line)
    {
    }
};

class BlockStatement {
public:
    std::vector<std::unique_ptr<Statement>> statements;
    int line;

    BlockStatement(std::vector<std::unique_ptr<Statement>> statements, int line)
        : statements(std::move(statements))
        , line(line)
    {
    }
};

class IfStatement {
public:
    std::unique_ptr<Expression> condition;
    std::unique_ptr<Statement> thenBranch;
    std::unique_ptr<Statement> elseBranch;
    int line;

    IfStatement(std::unique_ptr<Expression> condition,
        std::unique_ptr<Statement> thenBranch,
        std::unique_ptr<Statement> elseBranch,
        int line)
        : condition(std::move(condition))
        , thenBranch(std::move(thenBranch))
        , elseBranch(std::move(elseBranch))
        , line(line)
    {
    }
};

class WhileStatement {
public:
    std::unique_ptr<Expression> condition;
    std::unique_ptr<Statement> body;
    int line;

    WhileStatement(std::unique_ptr<Expression> condition, std::unique_ptr<Statement> body, int line)
        : condition(std::move(condition))
        , body(std::move(body))
        , line(line)
    {
    }
};

class ForStatement {
public:
    std::unique_ptr<Statement> initializer;
    std::unique_ptr<Expression> condition;
    std::unique_ptr<Expression> increment;
    std::unique_ptr<Statement> body;
    int line;

    ForStatement(std::unique_ptr<Statement> initializer,
        std::unique_ptr<Expression> condition,
        std::unique_ptr<Expression> increment,
        std::unique_ptr<Statement> body,
        const int line)
        : initializer(std::move(initializer))
        , condition(std::move(condition))
        , increment(std::move(increment))
        , body(std::move(body))
        , line(line)
    {
    }
};

class ReturnStatement {
public:
    Token keyword;
    std::unique_ptr<Expression> value;
    int line;

    ReturnStatement(Token keyword, std::unique_ptr<Expression> value, int line)
        : keyword(std::move(keyword))
        , value(std::move(value))
        , line(line)
    {
    }
};

class BreakStatement {
public:
    Token keyword;
    int line;

    BreakStatement(Token keyword, const int line)
        : keyword(std::move(keyword))
        , line(line)
    {
    }
};

class ContinueStatement {
public:
    Token keyword;
    int line;

    ContinueStatement(Token keyword, int line)
        : keyword(std::move(keyword))
        , line(line)
    {
    }
};

class FunctionDeclaration {
public:
    const Token name;
    std::vector<Token> parameters;
    std::unique_ptr<Statement> body;
    int line;

    FunctionDeclaration(Token name, std::vector<Token> parameters, std::unique_ptr<Statement> body, const int line)
        : name(std::move(name))
        , parameters(std::move(parameters))
        , body(std::move(body))
        , line(line)
    {
    }
};

class SwitchStatement {
public:
    std::unique_ptr<Expression> expression;
    std::vector<std::pair<std::unique_ptr<Expression>, std::unique_ptr<Statement>>> cases;
    std::unique_ptr<Statement> defaultCase;
    int line;

    SwitchStatement(std::unique_ptr<Expression> expression,
        std::vector<std::pair<std::unique_ptr<Expression>, std::unique_ptr<Statement>>> cases,
        std::unique_ptr<Statement> defaultCase,
        int line)
        : expression(std::move(expression))
        , cases(std::move(cases))
        , defaultCase(std::move(defaultCase))
        , line(line)
    {
    }
};

class Statement {
public:
    std::variant<ExpressionStatement, PrintStatement, VariableDeclaration, BlockStatement, IfStatement, WhileStatement, ForStatement, ReturnStatement, BreakStatement, ContinueStatement, FunctionDeclaration, SwitchStatement> as;
    int line;
    explicit Statement(std::variant<ExpressionStatement, PrintStatement, VariableDeclaration, BlockStatement, IfStatement, WhileStatement, ForStatement, ReturnStatement, BreakStatement, ContinueStatement, FunctionDeclaration, SwitchStatement> as, int line)
        : as { std::move(as) }
        , line { line }
    {
    }
};
