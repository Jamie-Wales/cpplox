#pragma once
#include <cstddef>
#include <string>
#include <variant>

class Obj;
struct ObjFunction;

struct Value {
    std::variant<double, bool, nullptr_t, Obj*> as;

    Value()
        : as(nullptr)
    {
    }

    Value(double value)
        : as(value)
    {
    }
    Value(bool value)
        : as(value)
    {
    }
    Value(nullptr_t null)
        : as(null)
    {
    }
    Value(Obj* obj)
        : as(obj)
    {
    }
    bool isString() const;
    bool isNumber() const;
    double asNumber() const;
    bool isTruthy() const;
    void print() const;
    std::string to_string() const;

    ObjFunction* asFunc() const;
    Value operator>=(const Value& other) const;
    Value operator<(const Value& other) const;
    Value operator*(const Value& other) const;
    Value operator+(const Value& other) const;
    Value operator==(const Value& other) const;
    Value& operator+=(const Value& other);
    Value operator*=(const Value& other);
    Value& operator/=(const Value& other);
    Value operator-() const;
    Value operator!() const;
    Value operator-(const Value& other) const;
    Value operator>(const Value& other) const;
};
