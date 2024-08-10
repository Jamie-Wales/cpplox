#include "Scanner.h"
#include <algorithm>
#include <iostream>
const std::vector<Scanner::RegexInfo> Scanner::regexList = {
    { std::regex(R"(\d+\.\d*f?)"), Tokentype::FLOAT },
    { std::regex(R"(\d+)"), Tokentype::INTEGER },
    { std::regex(R"("[^"]*")"), Tokentype::STRING },
    { std::regex(R"(<=)"), Tokentype::LESS_EQUAL },
    { std::regex(R"(>=)"), Tokentype::GREATER_EQUAL },
    { std::regex(R"(==)"), Tokentype::EQUAL_EQUAL },
    { std::regex(R"(!=)"), Tokentype::BANG_EQUAL },
    { std::regex(R"(<)"), Tokentype::LESS },
    { std::regex(R"(>)"), Tokentype::GREATER },
    { std::regex(R"(=)"), Tokentype::EQUAL },
    { std::regex(R"(\+)"), Tokentype::PLUS },
    { std::regex(R"(-)"), Tokentype::MINUS },
    { std::regex(R"(\*)"), Tokentype::STAR },
    { std::regex(R"(/)"), Tokentype::SLASH },
    { std::regex(R"(!)"), Tokentype::BANG },
    { std::regex(R"(\()"), Tokentype::LEFTPEREN },
    { std::regex(R"(\))"), Tokentype::RIGHTPEREN },
    { std::regex(R"(\{)"), Tokentype::LEFTBRACE },
    { std::regex(R"(\})"), Tokentype::RIGHTBRACE },
    { std::regex(R"(;)"), Tokentype::SEMICOLON },
    { std::regex(R"(\n|\r|$)"), Tokentype::CARRIGERETURN },
    { std::regex(R"(\s+)"), Tokentype::WHITESPACE },
};

const std::unordered_map<std::string, Tokentype> Scanner::keywords = {
    { "false", Tokentype::FALSE },
    { "true", Tokentype::TRUE },
    { "nil", Tokentype::NIL },
    // Add other keywords here
};

std::vector<Token> Scanner::tokenize()
{
    auto it = input.begin();
    const auto end = input.end();
    while (it != end) {
        if (auto token = matchToken(it)) {
            if (token->type == Tokentype::WHITESPACE) {
                handleWhitespace(token->lexeme);
            } else {
                tokens.push_back(std::move(*token));
            }
        } else {
            std::cerr << "Unexpected character: " << *it << std::endl;
            ++it;
        }
    }
    tokens.push_back(Token { Tokentype::EOF_TOKEN, "", line, row });
    return tokens;
}

std::optional<Token> Scanner::matchToken(std::string::const_iterator& it) const
{
    if (std::isalpha(*it) || *it == '_') {
        auto wordStart = it;
        while (it != input.end() && (std::isalnum(*it) || *it == '_')) {
            ++it;
        }
        std::string word(wordStart, it);

        auto keywordIt = keywords.find(word);
        if (keywordIt != keywords.end()) {
            return Token { keywordIt->second, word, line, row };
        }
        return Token { Tokentype::IDENTIFIER, word, line, row };
    }

    // Try regex matching for all other token types
    for (const auto& regex_info : regexList) {
        std::smatch match;
        if (std::regex_search(it, input.end(), match, regex_info.regex, std::regex_constants::match_continuous)) {
            std::string lexeme = match.str();
            it += lexeme.length();
            return Token { regex_info.type, lexeme, line, row };
        }
    }

    return std::nullopt;
}

void Scanner::handleWhitespace(const std::string& lexeme)
{
    for (char c : lexeme) {
        if (c == '\n') {
            ++line;
            row = 0;
        } else {
            ++row;
        }
    }
}
