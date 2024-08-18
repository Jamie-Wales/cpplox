#pragma once
#include "Visit.h"
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
};

struct ObjInstance {
};

class Obj {
public:
    std::variant<ObjString, ObjFunction, ObjInstance> as;

    template <typename T>
    explicit Obj(T value)
        : as(std::move(value))
    {
    }
    void print() const;
    [[nodiscard]] std::string to_string() const;
};
