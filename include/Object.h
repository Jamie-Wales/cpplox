#pragma once
#include <Visit.h>
#include <string>
#include <variant>

struct ObjString {
    const std::string* str;
    ObjString(const std::string* s)
        : str(s)
    {
    }
};

struct ObjFunction {
};

struct ObjInstance {
};

class Obj {
public:
    std::variant<ObjString, ObjFunction, ObjInstance> as;

    template <typename T>
    Obj(T value)
        : as(std::move(value))
    {
    }
    void print() const;
};
