// ScopeManager.cpp
#include "ScopeManager.h"

void ScopeManager::enterScope(bool isClosure)
{
    scopes.push_back(Scope { std::vector<Variable>(), isClosure });
}

void ScopeManager::exitScope()
{
    if (!scopes.empty()) {
        scopes.pop_back();
    }
}

void ScopeManager::markInitialized()
{
    if (!scopes.empty() && !scopes.back().variables.empty()) {
        auto& lastVariable = scopes.back().variables.back();
        lastVariable.depth = scopes.size() - 1;
    }
}

void ScopeManager::markInitialized(Variable& variable) const
{
    if (!scopes.empty() && variable.type == Variable::Type::Local) {
        variable.depth = scopes.size() - 1;
    }
}

bool ScopeManager::isGlobal(const std::string& name) const
{
    return globals.find(name) != globals.end();
}

ScopeManager::Variable ScopeManager::declareVariable(const Token& name, bool isReadOnly)
{
    if (scopes.empty()) {
        globals[name.lexeme] = { name, Variable::Type::Global, 0, isReadOnly, 0 };
        return globals[name.lexeme];
    }
    auto& currentScope = scopes.back();
    Variable var(name, Variable::Type::Local, static_cast<uint8_t>(currentScope.variables.size()), isReadOnly, scopes.size() - 1);
    currentScope.variables.push_back(var);
    return var;
}

std::optional<ScopeManager::Variable> ScopeManager::resolveVariable(const Token& name)
{
    if (const auto globalIt = globals.find(name.lexeme); globalIt != globals.end()) {
        return globalIt->second;
    }

    for (auto it = scopes.rbegin(); it != scopes.rend(); ++it) {
        for (const auto& var : it->variables) {
            if (var.name.lexeme == name.lexeme) {
                return var;
            }
        }
        if (it->isClosure) {
            Variable upvalue(name, Variable::Type::Upvalue, static_cast<uint8_t>(it->variables.size()), false, scopes.size() - 1 - std::distance(scopes.rbegin(), it));
            it->variables.push_back(upvalue);
            return upvalue;
        }
    }

    return std::nullopt;
}
