#pragma once
#include "Chunk.h"
#include "Object.h"
#include <cstdint>
#include <unordered_map>
#include <utility>

enum class vState { OK,
    BAD };

struct CallFrame {
    ObjFunction*
        function;
    size_t ip;
    size_t stackOffset;
};

class vMachine {
public:
    std::vector<CallFrame> frames;
    void call(ObjFunction* function, int argCount);
    bool callValue(Value callee, int argCount);
    explicit vMachine()
        : globals {}
        , stack {}
    {
    }

    vMachine(vMachine&&) = default;
    vMachine(const vMachine&) = default;
    vMachine& operator=(vMachine&&) = default;
    vMachine& operator=(const vMachine&) = default;
    ~vMachine() = default;
    Value readConstant();
    Value readConstantLong();
    std::unordered_map<std::string, Value> globals;
    void runtimeError(const std::string& error);
    [[nodiscard]] vState getState() const
    {
        return this->state;
    }
    void run();
    void execute();
    void load(ObjFunction* mainFunction);
    void defineNative(const std::string& name, NativeFn function);

private:
    vState state
        = vState::OK;
    static constexpr size_t FRAMES_MAX = 64;
    static constexpr size_t STACK_MAX = FRAMES_MAX * 256;
    int readShort();
    std::vector<Value> stack;
    void swap();
    void dup();
    void add();
    void mult();
    void div();
    void neg();
    void logicalAnd();
    void logicalOr();
    void logicalNot();
    void resetStack();
    void greater();
    void equal();
    void less();
    void greaterEqual();
    void lessEqual();
    size_t& ip();
    uint8_t readByte();
    Chunk& instructions() const;
    size_t offset();
    void ensureStackSize(size_t size, const char* opcode) const;
    CallFrame& frame();
};
