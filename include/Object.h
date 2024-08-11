#pragma once
#include <Visit.h>
#include <format>
#include <iostream>
#include <string>
#include <variant>

/* #TODO ADD CONSTANT STRING */
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

    void print() const
    {
        std::visit(overloaded {
                       [](const ObjString& s) {
                           std::cout << std::format("\"{}\"", *s.str);
                       },
                       [](const ObjFunction&) {
                           std::cout << "<function>";
                       },
                       [](const ObjInstance&) {
                           std::cout << "<instance>";
                       } },
            as);
    }
};
