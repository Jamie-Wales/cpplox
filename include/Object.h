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

    ObjFunction(std::string name, int arity, Chunk chunk)
        : name(std::move(name))
        , arity(arity)
        , chunk(std::move(chunk))
    {
    }
    ObjFunction()
        : name("")
        , arity(0)
        , chunk(Chunk {})
    {
    }
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

class Obj {
public:
    std::variant<ObjString, ObjFunction, ObjInstance, ObjNative> as;

    template <typename T>
    explicit Obj(T value)
        : as(std::move(value))
    {
    }

    void print() const;
    [[nodiscard]] std::string to_string() const;
};
