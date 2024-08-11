#pragma once
#include "Token.h"
#include <optional>
#include <regex>
#include <string>
#include <vector>

using regex = std::regex;

class Scanner {
public:
    Scanner(const std::string& input)
        : input(input)
    {
    }
    Scanner() = default;
    std::vector<Token> tokenize();

    void addEOFToken()
    {
        Token eof = { Tokentype::EOF_TOKEN, "\0", 0, 0 };
        tokens.push_back(eof);
    }

private:
    struct RegexInfo {
        std::regex regex;
        Tokentype type;
    };

    static const std::unordered_map<std::string, Tokentype> keywords;
    static const std::vector<RegexInfo> regexList;
    static const std::vector<std::pair<std::string, Tokentype>> operators;

    const std::string input;
    std::vector<Token> tokens;
    int line = 0;
    int row = 0;

    std::optional<Token> matchToken(std::string::const_iterator& it) const;
    void updatePosition(const std::string& lexeme);
};
