#pragma once
#include "Expression.h"

class Statement;

class ExpressionStatement {
public:
    std::unique_ptr<Expression> expression;
    ExpressionStatement(std::unique_ptr<Expression> expression)
        : expression(std::move(expression))
    {
    }
};

class PrintStatement {
public:
    std::unique_ptr<Expression> expression;

    PrintStatement(std::unique_ptr<Expression> expression)
        : expression(std::move(expression))
    {
    }
};

class VariableDeclaration {
public:
    Token name;
    std::unique_ptr<Expression> initializer;
    bool isConst;

    VariableDeclaration(const Token& name, std::unique_ptr<Expression> initializer, bool isConst)
        : name(name)
        , initializer(std::move(initializer))
        , isConst(isConst)
    {
    }
};

class BlockStatement {
public:
    std::vector<std::unique_ptr<Statement>> statements;

    BlockStatement(std::vector<std::unique_ptr<Statement>> statements)
        : statements(std::move(statements))
    {
    }
};

class IfStatement {
public:
    std::unique_ptr<Expression> condition;
    std::unique_ptr<Statement> thenBranch;
    std::unique_ptr<Statement> elseBranch;

    IfStatement(std::unique_ptr<Expression> condition,
        std::unique_ptr<Statement> thenBranch,
        std::unique_ptr<Statement> elseBranch)
        : condition(std::move(condition))
        , thenBranch(std::move(thenBranch))
        , elseBranch(std::move(elseBranch))
    {
    }
};

class WhileStatement {
public:
    std::unique_ptr<Expression> condition;
    std::unique_ptr<Statement> body;

    WhileStatement(std::unique_ptr<Expression> condition, std::unique_ptr<Statement> body)
        : condition(std::move(condition))
        , body(std::move(body))
    {
    }
};

class ForStatement {
public:
    std::unique_ptr<Statement> initializer;
    std::unique_ptr<Expression> condition;
    std::unique_ptr<Expression> increment;
    std::unique_ptr<Statement> body;

    ForStatement(std::unique_ptr<Statement> initializer,
        std::unique_ptr<Expression> condition,
        std::unique_ptr<Expression> increment,
        std::unique_ptr<Statement> body)
        : initializer(std::move(initializer))
        , condition(std::move(condition))
        , increment(std::move(increment))
        , body(std::move(body))
    {
    }
};

class ReturnStatement {
public:
    Token keyword;
    std::unique_ptr<Expression> value;

    ReturnStatement(const Token& keyword, std::unique_ptr<Expression> value)
        : keyword(keyword)
        , value(std::move(value))
    {
    }
};

class BreakStatement {
public:
    Token keyword;
    BreakStatement(const Token& keyword)
        : keyword(keyword)
    {
    }
};

class ContinueStatement {
public:
    Token keyword;
    ContinueStatement(const Token& keyword)
        : keyword(keyword)
    {
    }
};

class FunctionDeclaration {
public:
    const Token name;
    std::vector<Token> parameters;
    std::unique_ptr<Statement> body;
    FunctionDeclaration(const Token& name, std::vector<Token> parameters, std::unique_ptr<Statement> body)
        : name(name)
        , parameters(std::move(parameters))
        , body(std::move(body))
    {
    }
};

class SwitchStatement {
public:
    std::unique_ptr<Expression> expression;
    std::vector<std::pair<std::unique_ptr<Expression>, std::unique_ptr<Statement>>> cases;
    std::unique_ptr<Statement> defaultCase;
    SwitchStatement(std::unique_ptr<Expression> expression,
        std::vector<std::pair<std::unique_ptr<Expression>, std::unique_ptr<Statement>>> cases,
        std::unique_ptr<Statement> defaultCase)
        : expression(std::move(expression))
        , cases(std::move(cases))
        , defaultCase(std::move(defaultCase))
    {
    }
};

class Statement {
public:
    std::variant<ExpressionStatement, PrintStatement, VariableDeclaration, BlockStatement, IfStatement, WhileStatement, ForStatement, ReturnStatement, BreakStatement, ContinueStatement, FunctionDeclaration, SwitchStatement> as;
    Statement(std::variant<ExpressionStatement, PrintStatement, VariableDeclaration, BlockStatement, IfStatement, WhileStatement, ForStatement, ReturnStatement, BreakStatement, ContinueStatement, FunctionDeclaration, SwitchStatement> as)
        : as { std::move(as) } {};
};
