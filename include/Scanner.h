#pragma once
#include "Token.h"
#include <optional>
#include <regex>
#include <string>
#include <string_view>
#include <vector>

class Scanner {
public:
    Scanner(const std::string& input)
        : input(std::move(input))
    {
    }
    Scanner() = default;
    std::vector<Token> tokenize();

private:
    const std::string input;
    std::vector<Token> tokens;
    int line = 0;
    int row = 0;

    struct RegexInfo {
        std::regex regex;
        Tokentype type;
    };

    static const std::vector<RegexInfo> regexList;

    std::optional<Token> matchToken(std::string::const_iterator& it) const;
    void handleWhitespace(std::string_view lexeme);
};
