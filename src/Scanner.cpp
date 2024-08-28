#include "Scanner.h"
#include "Token.h"

std::vector<Token> Scanner::tokenize()
{
    std::vector<Token> allTokens;
    auto it = input.begin();
    const auto end = input.end();
    while (it != end) {
        if (auto token = matchToken(it)) {
            if (token->type == Tokentype::UNTERMINATED_STRING || token->type == Tokentype::UNTERMINATED_COMMENT) {
                throw std::runtime_error("Unterminated " + std::string(token->type == Tokentype::UNTERMINATED_STRING ? "string" : "comment") + " at line " + std::to_string(line) + ", column " + std::to_string(row));
            }
            if (token->type != Tokentype::WHITESPACE && token->type != Tokentype::CARRIGERETURN) {
                allTokens.push_back(std::move(*token));
            }
            updatePosition(token->lexeme);
        } else {
            throw std::runtime_error("Unexpected character at line " + std::to_string(line) + ", column " + std::to_string(row) + ": " + std::string(1, *it));
        }
    }
    allTokens.emplace_back(Tokentype::EOF_TOKEN, "", line, row);
    return allTokens;
}

std::optional<Token> Scanner::matchToken(std::string::const_iterator& it) const
{
    for (const auto& regex_info : regexList) {
        std::smatch match;
        if (std::regex_search(it, input.end(), match, regex_info.regex, std::regex_constants::match_continuous)) {
            const std::string lexeme = match.str();
            it += lexeme.length();
            Tokentype type = regex_info.type;
            if (type == Tokentype::IDENTIFIER) {
                if (const auto keywordIt = keywords.find(lexeme); keywordIt != keywords.end()) {
                    type = keywordIt->second;
                }
            }
            return Token { type, lexeme, line, row };
        }
    }
    return std::nullopt;
}

void Scanner::updatePosition(const std::string& lexeme)
{
    for (size_t i = 0; i < lexeme.length(); ++i) {
        if (lexeme[i] == '\n' || (lexeme[i] == '\r' && (i + 1 == lexeme.length() || lexeme[i + 1] != '\n'))) {
            ++line;
            row = 1;
        } else if (lexeme[i] == '\r' && i + 1 < lexeme.length() && lexeme[i + 1] == '\n') {
            ++line;
            row = 1;
            ++i;
        } else {
            ++row;
        }
    }
}

/* ---- dfa ---- */
const std::vector<Scanner::RegexInfo> Scanner::regexList = {
    { std::regex(R"(//.*(?:\n|$))"), Tokentype::COMMENT },
    { std::regex(R"(/\*(?:[^*]|\*(?!/))*\*/)"), Tokentype::COMMENT },
    { std::regex(R"(/\*(?:[^*]|\*(?!/))*$)"), Tokentype::UNTERMINATED_COMMENT },
    { std::regex(R"("(?:[^"\\]|\\.)*")"), Tokentype::STRING },
    { std::regex(R"('(?:[^'\\]|\\.)*')"), Tokentype::STRING },
    { std::regex(R"("(?:[^"\\]|\\.)*$)"), Tokentype::UNTERMINATED_STRING },
    { std::regex(R"('(?:[^'\\]|\\.)*$)"), Tokentype::UNTERMINATED_STRING },
    { std::regex(R"(\d+\.\d*f?)"), Tokentype::FLOAT },
    { std::regex(R"(\d+)"), Tokentype::INTEGER },
    { std::regex(R"(<=)"), Tokentype::LESS_EQUAL },
    { std::regex(R"(>=)"), Tokentype::GREATER_EQUAL },
    { std::regex(R"(==)"), Tokentype::EQUAL_EQUAL },
    { std::regex(R"(!=)"), Tokentype::BANG_EQUAL },
    { std::regex(R"(<)"), Tokentype::LESS },
    { std::regex(R"(>)"), Tokentype::GREATER },
    { std::regex(R"(=)"), Tokentype::EQUAL },
    { std::regex(R"(\*)"), Tokentype::STAR },
    { std::regex(R"(/)"), Tokentype::SLASH },
    { std::regex(R"(!)"), Tokentype::BANG },
    { std::regex(R"(\()"), Tokentype::LEFTPEREN },
    { std::regex(R"(\))"), Tokentype::RIGHTPEREN },
    { std::regex(R"(\{)"), Tokentype::LEFTBRACE },
    { std::regex(R"(\})"), Tokentype::RIGHTBRACE },
    { std::regex(R"(;)"), Tokentype::SEMICOLON },
    { std::regex(R"(\n|\r\n|\r)"), Tokentype::CARRIGERETURN },
    { std::regex(R"([ \t]+)"), Tokentype::WHITESPACE },
    { std::regex(R"([a-zA-Z_][a-zA-Z0-9_]*)"), Tokentype::IDENTIFIER },
    { std::regex(R"(\+\+)"), Tokentype::INCREMENT },
    { std::regex(R"(--)"), Tokentype::DECREMENT },
    { std::regex(R"(->)"), Tokentype::ARROW },
    { std::regex(R"(\+)"), Tokentype::PLUS },
    { std::regex(R"(-)"), Tokentype::MINUS },
    { std::regex("_"), Tokentype::UNDERSCORE },
    { std::regex(","), Tokentype::COMMA },
};

const std::unordered_map<std::string, Tokentype> Scanner::keywords = {
    { "false", Tokentype::FALSE },
    { "true", Tokentype::TRUE },
    { "nil", Tokentype::NIL },
    { "print", Tokentype::PRINT },
    { "let", Tokentype::LET },
    { "and", Tokentype::AND },
    { "or", Tokentype::OR },
    { "const", Tokentype::CONST },
    { "if", Tokentype::IF },
    { "else", Tokentype::ELSE },
    { "while", Tokentype::WHILE },
    { "for", Tokentype::FOR },
    { "continue", Tokentype::CONTINUE },
    { "break", Tokentype::BREAK },
    { "switch", Tokentype::SWITCH },
    { "return", Tokentype::RETURN },
    { "fn", Tokentype::FUN}
};
