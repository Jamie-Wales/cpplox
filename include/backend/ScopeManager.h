#pragma once
#include "Token.h"
#include <optional>
#include <vector>
#include <unordered_map>
#include <string>

class ScopeManager {
public:
    struct Variable {
        enum class Type { Local, Upvalue, Global };
        Token name;
        Type type;
        uint8_t index;
        bool isReadOnly;
        int depth;

        // Default constructor
        Variable() : type(Type::Local), index(0), isReadOnly(false), depth(0) {}

        Variable(const Token& name, Type type, uint8_t index, bool isReadOnly, int depth)
            : name(name), type(type), index(index), isReadOnly(isReadOnly), depth(depth) {}
    };

    struct Scope {
        std::vector<Variable> variables;
        bool isClosure;
    };

    std::vector<Scope> scopes;
    std::unordered_map<std::string, Variable> globals;

    void enterScope(bool isClosure = false);
    void exitScope();
    Variable declareVariable(const Token& name, bool isReadOnly);
    void markInitialized();
    void markInitialized(Variable& variable) const;
    std::optional<Variable> resolveVariable(const Token& name);
    bool isGlobal(const std::string& name) const;
};