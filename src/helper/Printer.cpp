
#include "Printer.h"
#include "Expression.h"
#include "Visit.h"
#include <memory>
#include <ranges>
#include <variant>

template <typename... Args>
void Printer::println(size_t depth, std::format_string<Args...> fmt, Args&&... args) const
{
    std::cout << std::string(depth * INDENT.size(), ' ')
              << std::format(fmt, std::forward<Args>(args)...) << '\n';
}

std::string Printer::join_tokens(const std::vector<Token>& tokens) const
{
    std::string result;
    for (const auto& token : tokens) {
        if (!result.empty())
            result += " ";
        result += token.lexeme;
    }
    return result;
}

void Printer::print(std::vector<std::unique_ptr<Statement>>& stmt)
{
    for (int i = 0; i < stmt.size(); i++) {
        print(*stmt[i], i);
    }
}

void Printer::print(const Expression& expr, size_t depth) const
{
    std::visit(overloaded {
                   [&](const LiteralExpression& e) { println(depth, "Literal: {}", e.value.lexeme); },
                   [&](const VariableExpression& e) {
                       println(depth, "Variable: {}", e.name.lexeme);
                       if (e.value) {
                           println(depth + 1, "Value:");
                           print(*e.value, depth + 2);
                       }
                   },
                   [&](const UnaryExpression& e) {
                       println(depth, "Unary: {}", e.operatorToken.lexeme);
                       print(*e.operand, depth + 1);
                   },
                   [&](const BinaryExpression& e) {
                       println(depth, "Binary: {}", e.operatorToken.lexeme);
                       print(*e.left, depth + 1);
                       print(*e.right, depth + 1);
                   },
                   [&](const AssignmentExpression& e) {
                       println(depth, "Assignment: {}", e.name.lexeme);
                       print(*e.value, depth + 1);
                   },
                   [&](const LogicalExpression& e) {
                       println(depth, "Logical: {}", e.operatorToken.lexeme);
                       print(*e.left, depth + 1);
                       print(*e.right, depth + 1);
                   },

                   [&](const IncrementExpression& e) {
                       e.postFix ? println(depth, "{}{}", e.name.lexeme, e.tokenOperator.lexeme) : println(depth, "{}{}", e.tokenOperator.lexeme, e.name.lexeme);
                   },
                   [&](const CallExpression& e) {
                       println(depth, "Call:");
                       println(depth + 1, "Callee:");
                       print(*e.callee, depth + 2);
                       println(depth + 1, "Arguments:");
                       for (const auto& arg : e.arguments) {
                           print(*arg, depth + 2);
                       }
                   } },
        expr.as);
}

void Printer::print(const Statement& stmt, size_t depth) const
{
    std::visit(overloaded {
                   [&](const ExpressionStatement& s) {
                       println(depth, "Expression Statement:");
                       print(*s.expression, depth + 1);
                   },
                   [&](const PrintStatement& s) {
                       println(depth, "Print Statement:");
                       print(*s.expression, depth + 1);
                   },
                   [&](const VariableDeclaration& s) {
                       println(depth, "Variable Declaration: {} {}", s.name.lexeme, s.isConst ? "(const)" : "");
                       if (s.initializer) {
                           println(depth + 1, "Initializer:");
                           print(*s.initializer, depth + 2);
                       }
                   },
                   [&](const BlockStatement& s) {
                       println(depth, "Block Statement:");
                       for (const auto& stmt : s.statements) {
                           print(*stmt, depth + 1);
                       }
                   },
                   [&](const IfStatement& s) {
                       println(depth, "If Statement:");
                       println(depth + 1, "Condition:");
                       print(*s.condition, depth + 2);
                       println(depth + 1, "Then Branch:");
                       print(*s.thenBranch, depth + 2);
                       if (s.elseBranch) {
                           println(depth + 1, "Else Branch:");
                           print(*s.elseBranch, depth + 2);
                       }
                   },
                   [&](const WhileStatement& s) {
                       println(depth, "While Statement:");
                       println(depth + 1, "Condition:");
                       print(*s.condition, depth + 2);
                       println(depth + 1, "Body:");
                       print(*s.body, depth + 2);
                   },
                   [&](const ForStatement& s) {
                       println(depth, "For Statement:");
                       if (s.initializer) {
                           println(depth + 1, "Initializer:");
                           print(*s.initializer, depth + 2);
                       }
                       if (s.condition) {
                           println(depth + 1, "Condition:");
                           print(*s.condition, depth + 2);
                       }
                       if (s.increment) {
                           println(depth + 1, "Increment:");
                           print(*s.increment, depth + 2);
                       }
                       println(depth + 1, "Body:");
                       print(*s.body, depth + 2);
                   },
                   [&](const ReturnStatement& s) {
                       println(depth, "Return Statement:");
                       if (s.value) {
                           print(*s.value, depth + 1);
                       }
                   },
                   [&](const BreakStatement& s) {
                       println(depth, "Break Statement");
                   },
                   [&](const ContinueStatement& s) {
                       println(depth, "Continue Statement");
                   },
                   [&](const FunctionDeclaration& s) {
                       println(depth, "Function Declaration: {}", s.name.lexeme);
                       println(depth + 1, "Parameters: {}", join_tokens(s.parameters));
                       println(depth + 1, "Body:");
                       print(*s.body, depth + 2);
                   },
                   [&](const SwitchStatement& s) {
                       println(depth, "Switch Statement:");
                       println(depth + 1, "Expression:");
                       print(*s.expression, depth + 2);
                       println(depth + 1, "Cases:");
                       for (const auto& [caseExpr, caseStmt] : s.cases) {
                           println(depth + 2, "Case:");
                           print(*caseExpr, depth + 3);
                           print(*caseStmt, depth + 3);
                       }
                       if (s.defaultCase) {
                           println(depth + 2, "Default Case:");
                           print(*s.defaultCase, depth + 3);
                       }
                   } },
        stmt.as);
};
