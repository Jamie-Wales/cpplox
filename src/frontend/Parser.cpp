#include "Parser.h"
#include "Expression.h"
#include "Statement.h"
#include <cstddef>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <vector>

void Parser::advance()
{
    previous = tokens[current];
    if (current < tokens.size()) {
        current++;
    }
}

Token Parser::previousToken()
{
    return previous;
}

bool Parser::consume(Tokentype type, const std::string& message)
{
    if (check(type)) {
        advance();
        return true;
    }
    errorAtCurrent(message);
    return false;
}

void Parser::errorAtCurrent(const std::string& message)
{
    errorAt(peek(), message);
}

void Parser::error(const std::string& message)
{
    errorAt(previousToken(), message);
}
void Parser::initRules()
{

    rules[Tokentype::IDENTIFIER] = { &Parser::variable, nullptr, nullptr, Precedence::NONE };
    rules[Tokentype::LEFTPEREN] = { &Parser::grouping, &Parser::call, nullptr, Precedence::CALL };
    rules[Tokentype::MINUS] = { &Parser::unary, &Parser::binary, nullptr, Precedence::TERM };
    rules[Tokentype::PLUS] = { nullptr, &Parser::binary, nullptr, Precedence::TERM };
    rules[Tokentype::SLASH] = { nullptr, &Parser::binary, nullptr, Precedence::FACTOR };
    rules[Tokentype::STAR] = { nullptr, &Parser::binary, nullptr, Precedence::FACTOR };
    rules[Tokentype::INTEGER] = { &Parser::literal, nullptr, nullptr, Precedence::NONE };
    rules[Tokentype::STRING] = { &Parser::literal, nullptr, nullptr, Precedence::NONE };
    rules[Tokentype::TRUE] = { &Parser::literal, nullptr, nullptr, Precedence::NONE };
    rules[Tokentype::FALSE] = { &Parser::literal, nullptr, nullptr, Precedence::NONE };
    rules[Tokentype::NIL] = { &Parser::literal, nullptr, nullptr, Precedence::NONE };
    rules[Tokentype::BANG] = { &Parser::unary, nullptr, nullptr, Precedence::UNARY };
    rules[Tokentype::EQUAL_EQUAL] = { nullptr, &Parser::binary, nullptr, Precedence::EQUALITY };
    rules[Tokentype::BANG_EQUAL] = { nullptr, &Parser::binary, nullptr, Precedence::EQUALITY };
    rules[Tokentype::LESS] = { nullptr, &Parser::binary, nullptr, Precedence::COMPARISON };
    rules[Tokentype::LESS_EQUAL] = { nullptr, &Parser::binary, nullptr, Precedence::COMPARISON };
    rules[Tokentype::GREATER] = { nullptr, &Parser::binary, nullptr, Precedence::COMPARISON };
    rules[Tokentype::GREATER_EQUAL] = { nullptr, &Parser::binary, nullptr, Precedence::COMPARISON };
    rules[Tokentype::AND] = { nullptr, &Parser::and_, nullptr, Precedence::AND };
    rules[Tokentype::OR] = { nullptr, &Parser::or_, nullptr, Precedence::OR };
    rules[Tokentype::INCREMENT] = { nullptr, nullptr, &Parser::postfix, Precedence::NONE };
}

std::unique_ptr<Expression> Parser::or_(std::unique_ptr<Expression> left, bool canAssign)
{
    Token operatorToken = previousToken();
    auto right = parsePrecedence(Precedence::OR);
    return std::make_unique<Expression>(LogicalExpression { std::move(left), operatorToken, std::move(right), operatorToken.line });
}

void Parser::errorAt(const Token& token, const std::string& message)
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

bool Parser::match(Tokentype type)
{
    if (check(type)) {
        advance();
        return true;
    }
    return false;
}

bool Parser::check(Tokentype type) const
{
    if (current >= tokens.size())
        return false;
    return tokens[current].type == type;
}

Token Parser::peek() const
{
    if (current >= tokens.size()) {
        return tokens.back();
    }
    return tokens[current];
}

void Parser::synchronize()
{
    panicMode = false;

    while (peek().type != Tokentype::EOF_TOKEN) {
        if (previousToken().type == Tokentype::SEMICOLON)
            return;

        switch (peek().type) {
        case Tokentype::CLASS:
        case Tokentype::FUN:
        case Tokentype::LET:
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

Parser::ParseRule Parser::getRule(Tokentype type)
{
    auto it = rules.find(type);
    if (it != rules.end()) {
        return it->second;
    }
    return { nullptr, nullptr, nullptr, Precedence::NONE };
}

std::unique_ptr<Expression> Parser::expression()
{
    return parsePrecedence(Precedence::ASSIGNMENT);
}

std::unique_ptr<Expression> Parser::grouping(bool canAssign)

{
    auto expr = expression();
    consume(Tokentype::RIGHTPEREN, "Expect ')' after expression.");
    return expr;
}

std::unique_ptr<Expression> Parser::unary(bool canAssign)
{
    Token operatorToken = previousToken();
    auto right = parsePrecedence(Precedence::UNARY);
    return std::make_unique<Expression>(UnaryExpression { operatorToken, std::move(right), operatorToken.line });
}

std::unique_ptr<Expression> Parser::parsePrecedence(Precedence precedence)
{
    advance();
    ParseFn prefixRule = getRule(previousToken().type).prefix;
    if (prefixRule == nullptr) {
        error("Expect expression.");
        return nullptr;
    }

    bool canAssign = precedence <= Precedence::ASSIGNMENT;
    auto left = (this->*prefixRule)(canAssign);

    while (precedence <= getRule(peek().type).precedence) {
        advance();
        InfixFn infixRule = getRule(previousToken().type).infix;
        if (infixRule != nullptr) {
            left = (this->*infixRule)(std::move(left), canAssign);
        } else {
            break;
        }
    }

    if (canAssign && match(Tokentype::EQUAL)) {
        error("Invalid assignment target.");
    }

    return left;
}

std::unique_ptr<Expression> Parser::variable(bool canAssign)
{
    Token name = previousToken();

    if (canAssign && match(Tokentype::EQUAL)) {
        auto value = expression();
        return std::make_unique<Expression>(AssignmentExpression { name, std::move(value), name.line });
    } else {
        return std::make_unique<Expression>(VariableExpression { name, name.line });
    }
}
std::unique_ptr<Expression> Parser::binary(std::unique_ptr<Expression> left, bool canAssign)
{
    Token operatorToken = previousToken();
    ParseRule rule = getRule(operatorToken.type);
    auto right = parsePrecedence(static_cast<Precedence>(static_cast<int>(rule.precedence) + 1));
    return std::make_unique<Expression>(BinaryExpression { std::move(left), operatorToken, std::move(right), operatorToken.line });
}
std::unique_ptr<Expression> Parser::literal(bool canAssign)
{
    const Token& previous = previousToken();
    return std::make_unique<Expression>(LiteralExpression { previous, previous.line });
}

std::unique_ptr<Expression> Parser::and_(std::unique_ptr<Expression> left, bool canAssign)
{
    Token operatorToken = previousToken();
    auto right = parsePrecedence(Precedence::OR);
    return std::make_unique<Expression>(LogicalExpression { std::move(left), operatorToken, std::move(right), operatorToken.line });
}

std::unique_ptr<Expression> Parser::prefix(bool canAssign)
{
    Token operatorToken = previousToken();
    auto operand = parsePrecedence(Precedence::UNARY);
    return std::make_unique<Expression>(UnaryExpression { operatorToken, std::move(operand), operatorToken.line });
}

std::unique_ptr<Expression> Parser::postfix(std::unique_ptr<Expression> left, bool canAssign)
{
    Token operatorToken = previousToken();
    return std::make_unique<Expression>(UnaryExpression { operatorToken, std::move(left), operatorToken.line });
}

std::unique_ptr<Statement> Parser::declaration()
{
    try {
        if (match(Tokentype::LET) || match(Tokentype::CONST)) {
            return variableDeclaration();
        }
        if (match(Tokentype::FUN)) {
            return functionDeclaration("function");
        }
        return statement();
    } catch (std::exception& e) {
        synchronize();
        std::cerr << e.what() << std::endl;
        return nullptr;
    }
}

std::unique_ptr<Statement> Parser::blockStatement()
{
    std::vector<std::unique_ptr<Statement>> statements;

    while (!check(Tokentype::RIGHTBRACE) && !check(Tokentype::EOF_TOKEN)) {
        statements.push_back(declaration());
    }
    consume(Tokentype::RIGHTBRACE, "Expect '}' after block.");

    return std::make_unique<Statement>(BlockStatement { std::move(statements), previousToken().line });
}

std::unique_ptr<Statement> Parser::printStatement()
{
    auto value = expression();
    return std::make_unique<Statement>(PrintStatement { std::move(value), previousToken().line });
}

std::unique_ptr<Statement> Parser::statement()
{
    if (match(Tokentype::PRINT)) {
        return printStatement();
    } else if (match(Tokentype::IF)) {
        return ifStatement();
    } else if (match(Tokentype::WHILE)) {
        return whileStatement();
    } else if (match(Tokentype::FOR)) {
        return forStatement();
    } else if (match(Tokentype::RETURN)) {
        return returnStatement();
    } else if (match(Tokentype::LEFTBRACE)) {
        return blockStatement();
    } else {
        return expressionStatement();
    }
}

std::unique_ptr<Statement> Parser::expressionStatement()
{
    auto expr = expression();
    consume(Tokentype::SEMICOLON, "Expect ';' after expression.");
    return std::make_unique<Statement>(ExpressionStatement { std::move(expr), previousToken().line });
}
std::unique_ptr<Statement> Parser::ifStatement()
{
    consume(Tokentype::LEFTPEREN, "Expect '(' after 'if'.");
    int line = previousToken().line;
    auto condition = expression();
    consume(Tokentype::RIGHTPEREN, "Expect ')' after condition.");

    auto thenBranch = statement();
    std::unique_ptr<Statement> elseBranch = nullptr;
    if (match(Tokentype::ELSE)) {
        elseBranch = statement();
    }

    return std::make_unique<Statement>(IfStatement { std::move(condition), std::move(thenBranch), std::move(elseBranch), line });
}

std::unique_ptr<Statement> Parser::whileStatement()
{
    consume(Tokentype::LEFTPEREN, "Expect '(' after 'while'.");
    int line = previousToken().line;
    auto condition = expression();
    consume(Tokentype::RIGHTPEREN, "Expect ')' after condition.");

    auto body = statement();

    return std::make_unique<Statement>(WhileStatement { std::move(condition), std::move(body), line });
}

std::unique_ptr<Statement> Parser::forStatement()
{
    consume(Tokentype::LEFTPEREN, "Expect '(' after 'for'.");
    int line = previousToken().line;
    std::unique_ptr<Statement> initializer;
    if (match(Tokentype::SEMICOLON)) {
        initializer = nullptr;
    } else if (match(Tokentype::LET) || match(Tokentype::CONST)) {
        initializer = variableDeclaration();
    } else {
        initializer = expressionStatement();
    }

    std::unique_ptr<Expression> condition = nullptr;
    if (!check(Tokentype::SEMICOLON)) {
        condition = expression();
    }
    consume(Tokentype::SEMICOLON, "Expect ';' after loop condition.");

    std::unique_ptr<Expression> increment = nullptr;
    if (!check(Tokentype::RIGHTPEREN)) {
        increment = expression();
    }
    consume(Tokentype::RIGHTPEREN, "Expect ')' after for clauses.");

    auto body = statement();

    return std::make_unique<Statement>(ForStatement { std::move(initializer), std::move(condition), std::move(increment), std::move(body), line });
}

std::unique_ptr<Statement> Parser::returnStatement()
{
    const Token keyword = previousToken();
    std::unique_ptr<Expression> value = nullptr;
    if (!check(Tokentype::SEMICOLON)) {
        value = expression();
    }
    consume(Tokentype::SEMICOLON, "Expect ';' after return value.");
    int line = previousToken().line;
    return std::make_unique<Statement>(ReturnStatement { keyword, std::move(value), line });
}

std::unique_ptr<Statement> Parser::variableDeclaration()
{
    const Token type  = previousToken();
    const bool isConst = type.type == Tokentype::CONST;
    consume(Tokentype::IDENTIFIER, "Expect variable name.");
    const Token& prev = previousToken();

    std::unique_ptr<Expression> initializer = nullptr;
    if (match(Tokentype::EQUAL)) {
        initializer = expression();
    } else if (isConst) {
        throw std::runtime_error("Const Variables must be initialized");
    }
    consume(Tokentype::SEMICOLON, "Expect ';' after variable declaration.");
    return std::make_unique<Statement>(VariableDeclaration { prev, std::move(initializer), isConst, prev.line });
}

std::unique_ptr<Statement> Parser::functionDeclaration(const std::string& kind)
{
    consume(Tokentype::IDENTIFIER, "Expect " + kind + " name.");
    Token name = previousToken();
    consume(Tokentype::LEFTPEREN, "Expect '(' after " + kind + " name.");

    std::vector<Token> parameters;
    if (!check(Tokentype::RIGHTPEREN)) {
        do {
            if (parameters.size() >= 255) {
                error("Can't have more than 255 parameters.");
            }
            consume(Tokentype::IDENTIFIER, "Expect parameter name.");
            parameters.push_back(previousToken());
        } while (match(Tokentype::COMMA));
    }
    consume(Tokentype::RIGHTPEREN, "Expect ')' after parameters.");

    consume(Tokentype::LEFTBRACE, "Expect '{' before " + kind + " body.");
    auto body = blockStatement();
    return std::make_unique<Statement>(FunctionDeclaration { name, std::move(parameters), std::move(body), name.line });
}

std::unique_ptr<Expression> Parser::call(std::unique_ptr<Expression> callee, bool canAssign)
{
    std::vector<std::unique_ptr<Expression>> arguments;
    if (!check(Tokentype::RIGHTPEREN)) {
        do {
            if (arguments.size() >= 255) {
                error("Can't have more than 255 arguments.");
            }
            arguments.push_back(expression());
        } while (match(Tokentype::COMMA));
    }
    consume(Tokentype::RIGHTPEREN, "Expect ')' after arguments.");
    return std::make_unique<Expression>(CallExpression { std::move(callee), std::move(arguments), previousToken().line });
}
std::vector<std::unique_ptr<Statement>> Parser::parseProgram()
{
    std::vector<std::unique_ptr<Statement>> statements;
    while (!check(Tokentype::EOF_TOKEN)) {
        auto decl = declaration();
        if (decl != nullptr) {
            statements.push_back(std::move(decl));
        }
    }
    return statements;
}
