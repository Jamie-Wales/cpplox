#pragma once
#include "Token.h"
#include <memory>
#include <variant>
#include <vector>

class Expression;

class LiteralExpression {
public:
    Token value;
    int line;

    LiteralExpression(const Token& value, int line)
        : value { value }
        , line { line }
    {
    }
};

class VariableExpression {
public:
    Token name;
    std::unique_ptr<Expression> value;
    int line;

    VariableExpression(const Token& name, int line)
        : name { name }
        , line { line }
    {
    }
};

class UnaryExpression {
public:
    Token operatorToken;
    std::unique_ptr<Expression> operand;
    int line;

    UnaryExpression(const Token& operatorToken, std::unique_ptr<Expression> operand, int line)
        : operatorToken { operatorToken }
        , operand { std::move(operand) }
        , line { line }
    {
    }
};

class BinaryExpression {
public:
    std::unique_ptr<Expression> left;
    Token operatorToken;
    std::unique_ptr<Expression> right;
    int line;

    BinaryExpression(std::unique_ptr<Expression> left, const Token& operatorToken, std::unique_ptr<Expression> right, int line)
        : left { std::move(left) }
        , operatorToken { operatorToken }
        , right { std::move(right) }
        , line { line }
    {
    }
};

class AssignmentExpression {
public:
    Token name;
    std::unique_ptr<Expression> value;
    int line;

    AssignmentExpression(const Token& name, std::unique_ptr<Expression> value, int line)
        : name { name }
        , value { std::move(value) }
        , line { line }
    {
    }
};

class LogicalExpression {
public:
    std::unique_ptr<Expression> left;
    Token operatorToken;
    std::unique_ptr<Expression> right;
    int line;

    LogicalExpression(std::unique_ptr<Expression> left, const Token& operatorToken, std::unique_ptr<Expression> right, int line)
        : left { std::move(left) }
        , operatorToken { operatorToken }
        , right { std::move(right) }
        , line { line }
    {
    }
};

class CallExpression {
public:
    std::unique_ptr<Expression> callee;
    std::vector<std::unique_ptr<Expression>> arguments;
    int line;

    CallExpression(std::unique_ptr<Expression> callee, std::vector<std::unique_ptr<Expression>> arguments, int line)
        : callee { std::move(callee) }
        , arguments { std::move(arguments) }
        , line { line }
    {
    }
};

class IncrementExpression {
public:
    Token name;
    std::unique_ptr<Expression> element;
    Token tokenOperator;
    bool postFix;
    IncrementExpression(Token name, std::unique_ptr<Expression> element, const Token& tokenOperator, bool postFix)
        : name { std::move(name) }
        , element { std::move(element) }
        , tokenOperator { tokenOperator }
        , postFix { postFix }
    {
    }
};

class Expression {
public:
    std::variant<LiteralExpression, VariableExpression, UnaryExpression, BinaryExpression, AssignmentExpression, LogicalExpression, CallExpression, IncrementExpression> as;
    int line;

    explicit Expression(std::variant<LiteralExpression, VariableExpression, UnaryExpression, BinaryExpression, AssignmentExpression, LogicalExpression, CallExpression, IncrementExpression> as, int line)
        : as { std::move(as) }
        , line { line } {};
};
