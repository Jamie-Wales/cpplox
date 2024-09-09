#pragma once
#include "Chunk.h"
#include <functional>
#include <string>
#include <variant>

struct ObjString {
    const std::string* str;
    explicit ObjString(const std::string* s)
        : str(s)
    {
    }
};
struct ObjFunction {
    std::string name;
    int arity;
    Chunk chunk;
    size_t upValueCount;
    ObjFunction(std::string name, int arity, Chunk chunk)
        : name { std::move(name) }
        , arity { arity }
        , chunk { std::move(chunk) }
        , upValueCount { 0 }
    {
    }
    ObjFunction()
        : name("")
        , arity(0)
        , chunk(Chunk {})
        , upValueCount { 0 }
    {
    }
};

struct ObjUpvalue {
    Value* location;
};

class Value;

using NativeFn = std::function<Value(int argCount, Value* args)>;

struct ObjNative {
    NativeFn function;
    explicit ObjNative(NativeFn fn)
        : function(std::move(fn))
    {
    }
};

struct ObjInstance {
};

struct ObjClosure {
    ObjFunction* pFunction;
    std::vector<ObjUpvalue*> upValues;
    size_t size = 0;

    explicit ObjClosure(ObjFunction* pFunction)
    {
        this->pFunction = pFunction;
        upValues = {};
        upValues.reserve(pFunction->upValueCount);
        size = pFunction->upValueCount;
    }

};

class Obj {
public:
    std::variant<ObjString, ObjFunction, ObjInstance, ObjNative, ObjClosure> as;

    template <typename T>
    explicit Obj(T value)
        : as(std::move(value))
    {
    }

    void print() const;
    [[nodiscard]] std::string to_string() const;
};
