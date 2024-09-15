#pragma once

#include "Token.h"
#include <memory>
#include <vector>
class Expression;

class LiteralExpression {
public:
    Token value;

    LiteralExpression(const Token& value)
        : value { value } {};
};

class VariableExpression {
public:
    Token name;
    std::unique_ptr<Expression> value;
    VariableExpression(const Token& name, std::unique_ptr<Expression> expr)
        : name { name }
        , value { std::move(expr) }
    {
    }
};

class UnaryExpression {
public:
    Token operatorToken;
    std::unique_ptr<Expression> operand;
    UnaryExpression(const Token& operatorToken, std::unique_ptr<Expression> operand)
        : operatorToken(operatorToken)
        , operand(std::move(operand))
    {
    }
};

class BinaryExpression {
public:
    std::unique_ptr<Expression> left;
    Token operatorToken;
    std::unique_ptr<Expression> right;
    BinaryExpression(std::unique_ptr<Expression> left, const Token& operatorToken, std::unique_ptr<Expression> right)
        : left(std::move(left))
        , operatorToken(operatorToken)
        , right(std::move(right))
    {
    }
};

class AssignmentExpression {
public:
    Token name;
    std::unique_ptr<Expression> value;
    AssignmentExpression(const Token& name, std::unique_ptr<Expression> value)
        : name(name)
        , value(std::move(value))
    {
    }
};

class LogicalExpression {
public:
    std::unique_ptr<Expression> left;
    Token operatorToken;
    std::unique_ptr<Expression> right;

    LogicalExpression(std::unique_ptr<Expression> left, const Token& operatorToken, std::unique_ptr<Expression> right)
        : left(std::move(left))
        , operatorToken(operatorToken)
        , right(std::move(right))
    {
    }
};

class CallExpression {
public:
    std::unique_ptr<Expression> callee;
    std::vector<std::unique_ptr<Expression>> arguments;
    CallExpression(std::unique_ptr<Expression> callee, std::vector<std::unique_ptr<Expression>> arguments)
        : callee(std::move(callee))
        , arguments(std::move(arguments))
    {
    }
};

class Expression {

public:
    std::variant<LiteralExpression, VariableExpression, UnaryExpression, BinaryExpression, AssignmentExpression, LogicalExpression, CallExpression> as;

    explicit Expression(std::variant<LiteralExpression, VariableExpression, UnaryExpression, BinaryExpression, AssignmentExpression, LogicalExpression, CallExpression> as)
        : as { std::move(as) } {};
};
