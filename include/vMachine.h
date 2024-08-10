#pragma once
#include "Chunk.h"
#include <iostream>
#include <ostream>
#include <sstream>
#include <stack>
#include <string>

enum class vState { OK,
    BAD };

class vMachine {
public:
    vMachine(Chunk chunk)
        : instructions(chunk)
    {
    }
    vMachine(vMachine&&) = default;
    vMachine(const vMachine&) = default;
    vMachine& operator=(vMachine&&) = default;
    vMachine& operator=(const vMachine&) = default;
    ~vMachine() = default;
    Value readConstant();
    Value readConstantLong();

    void resetStack()
    {
        stack = std::stack<Value> {};
    }
    template <typename... Args>
    void runtimeError(const char* format, Args&&... args)
    {
        std::ostringstream error_stream;
        (error_stream << ... << std::forward<Args>(args));
        std::string error_message = error_stream.str();

        std::cerr << error_message << '\n';

        size_t instruction = ip - 1;
        int line = instructions.lines[instruction].lineNumber;
        std::cerr << "[line " << line << "] in script\n";
        resetStack();
    }
    vState getState()
    {
        return this->state;
    }
    void run();

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
};
