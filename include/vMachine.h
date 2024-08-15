#pragma once
#include "Chunk.h"
#include <stack>
#include <utility>

enum class vState { OK,
    BAD };

class vMachine {
public:
    explicit vMachine(Chunk chunk)
        : instructions(std::move(chunk))
    {
        globals = {};
    }
    vMachine() : globals{}, instructions{}, ip(0), stack{} {}
    vMachine(vMachine&&) = default;
    vMachine(const vMachine&) = default;
    vMachine& operator=(vMachine&&) = default;
    vMachine& operator=(const vMachine&) = default;
    ~vMachine() = default;
    Value readConstant();
    Value readConstantLong();
    std::unordered_map<std::string, Value> globals;

    void resetStack()
    {
        stack = {};
    }
    template <typename... Args>
    void runtimeError(const char* format, Args&&... args);
    [[nodiscard]] vState getState() const {
        return this->state;
    }
    void run();
    void execute(const Chunk &newInstructions);


private:
    vState state = vState::OK;
    Chunk instructions;
    int ip = 0;
    std::stack<Value> stack;
    void add();
    void sub();
    void mult();
    void div();
    void neg();
    void ensureStackSize(size_t size, const char* opcode);

};
