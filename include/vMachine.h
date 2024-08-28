#pragma once
#include "Chunk.h"
#include "Object.h"
#include <unordered_map>
#include <utility>

enum class vState { OK,
    BAD };

struct CallFrame {
    ObjFunction* function;
    size_t ip;
    size_t stackOffset;
};

class vMachine {
public:
    std::vector<CallFrame> frames;
    CallFrame* frame;

    void call(ObjFunction* function, int argCount);
    bool callValue(Value callee, int argCount);
    explicit vMachine(Chunk chunk)
            : globals{}
            , instructions(std::move(chunk))
            , ip(0)
            , stack{}
    {
        auto* mainFunction = new ObjFunction("main", 0, instructions);
        frames.push_back({mainFunction, 0, 0});
        frame = &frames.back();
    }

    vMachine()
            : globals{}
            , instructions{}
            , ip(0)
            , stack{}
    {
        auto* mainFunction = new ObjFunction("main", 0, Chunk{});
        frames.push_back({mainFunction, 0, 0});
        frame = &frames.back();
    }
    vMachine(vMachine&&) = default;
    vMachine(const vMachine&) = default;
    vMachine& operator=(vMachine&&) = default;
    vMachine& operator=(const vMachine&) = default;
    ~vMachine() = default;
    Value readConstant();
    Value readConstantLong();
    std::unordered_map<std::string, Value> globals;
    template <typename... Args>
    void runtimeError(const char* format, Args&&... args);
    [[nodiscard]] vState getState() const
    {
        return this->state;
    }
    void run();
    void execute(const Chunk& newInstructions);

    void defineNative(const std::string& name, NativeFn function);

private:
    vState state
        = vState::OK;
    Chunk instructions;
    int ip = 0;
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
    uint8_t readByte();
    void ensureStackSize(size_t size, const char* opcode) const;
};
