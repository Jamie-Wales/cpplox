#include <cstdlib>
#include <functional>

enum class Precedence {
    NONE,
    ASSIGNMENT,
    OR,
    AND,
    EQUALITY,
    COMPARISON,
    TERM,
    FACTOR,
    UNARY,
    CALL,
    PRIMARY
};

using ParseFn = std::function<void()>;

struct ParseRule {
    ParseFn prefix;
    ParseFn infix;
    Precedence precedence;
};

class Parser {
};
