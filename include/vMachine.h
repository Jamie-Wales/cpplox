#pragma once
#include "Chunk.h"
#include "Object.h"
#include "stdlibfuncs.h"
#include <cstdint>
#include <unordered_map>
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
    template <typename... Args>
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
    void defineNativeFunctions()
    {
        this->defineNative("abs", absNative);
        this->defineNative("pow", powNative);
        this->defineNative("sqrt", sqrtNative);
        this->defineNative("floor", floorNative);
        this->defineNative("ceil", ceilNative);
        this->defineNative("round", roundNative);
        this->defineNative("random", randomNative);
        this->defineNative("isNumber", isNumberNative);
        this->defineNative("isString", isStringNative);
        this->defineNative("isNull", isNullNative);
        this->defineNative("isBool", isBoolNative);
        this->defineNative("toNumber", toNumberNative);
        this->defineNative("toBoolean", toBooleanNative);
        this->defineNative("printNative", printNative);
        this->defineNative("input", inputNative);
        this->defineNative("length", lengthNative);
        this->defineNative("clock", clockNative);
    }
    vState state
        = vState::OK;
    static constexpr size_t FRAMES_MAX = 64;
    static constexpr size_t STACK_MAX = FRAMES_MAX * 256;
    int16_t readShort();
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

    void runtimeError(const std::string& error);

    size_t offset();
    void ensureStackSize(size_t size, const char* opcode) const;
    CallFrame& frame();
};
