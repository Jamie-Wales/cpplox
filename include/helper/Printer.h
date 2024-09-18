#include "Expression.h"
#include "Statement.h"
#include <format>
#include <iostream>
#include <memory>
#include <string>
#include <string_view>

class Printer {
private:
    static constexpr std::string_view INDENT = "  ";

    template <typename... Args>
    void println(size_t depth, std::format_string<Args...> fmt, Args&&... args) const;
    std::string join_tokens(const std::vector<Token>& tokens) const;

public:
    void print(std::vector<std::unique_ptr<Statement>>& stmt);
    void print(const Expression& expr, size_t depth = 0) const;
    void print(const Statement& stmt, size_t depth = 0) const;
};
