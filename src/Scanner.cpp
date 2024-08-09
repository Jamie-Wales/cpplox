#include "Scanner.h"
#include <iostream>

const std::vector<Scanner::RegexInfo> Scanner::regexList = {
    { std::regex(R"(\d+\.\d*f?)"), Tokentype::FLOAT },
    { std::regex(R"(\d+)"), Tokentype::INTEGER },
    { std::regex(R"("[^"]*")"), Tokentype::STRING },
    { std::regex(R"(\+)"), Tokentype::PLUS },
    { std::regex(R"(-)"), Tokentype::MINUS },
    { std::regex(R"(\*)"), Tokentype::STAR },
    { std::regex(R"(/)"), Tokentype::SLASH },
    { std::regex(R"(\))"), Tokentype::RIGHTPEREN },
    { std::regex(R"(\})"), Tokentype::RIGHTBRACE },
    { std::regex(R"(\()"), Tokentype::LEFTPEREN },
    { std::regex(R"(\{)"), Tokentype::LEFTBRACE },
    { std::regex(R"(;)"), Tokentype::SEMICOLON },
    { std::regex(R"([a-zA-Z_]\w*)"), Tokentype::IDENTIFIER },
    { std::regex(R"(\n|\r|$)"), Tokentype::CARRIGERETURN },
    { std::regex(R"(\s+)"), Tokentype::WHITESPACE },
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
    for (const auto& regex_info : regexList) {
        std::smatch match;
        if (std::regex_search(it, input.end(), match, regex_info.regex, std::regex_constants::match_continuous)) {
            std::string lexeme = match.str();
            it += lexeme.length();
            auto token = Token { regex_info.type, lexeme, line, row };
            return token;
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
