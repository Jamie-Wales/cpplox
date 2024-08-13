#pragma once
#include "Object.h"
#include <cstddef>
#include <variant>

struct Value {
    std::variant<double, bool, nullptr_t, Obj*> as;
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
    ~Value()
    {
        if (auto obj_ptr = std::get_if<Obj*>(&as)) {
            delete *obj_ptr;
        }
    }
    Value(const Value& other)
        : as(other.as)
    {
        if (auto obj_ptr = std::get_if<Obj*>(&as)) {
            as = new Obj(**obj_ptr);
        }
    }
    Value(Value&& other) noexcept
        : as(std::move(other.as))
    {
        if (auto obj_ptr = std::get_if<Obj*>(&other.as)) {
            *obj_ptr = nullptr;
        }
    }
    Value& operator=(const Value& other);
    Value& operator=(Value&& other) noexcept;
    void print() const;
    Value operator+(const Value& other) const;
    Value operator==(const Value& other) const;
    Value& operator+=(const Value& other);
    Value& operator*=(const Value& other);
    Value& operator/=(const Value& other);
    Value operator-() const;
    Value operator!() const;
    Value operator-(const Value& other) const;
    Value operator>(const Value& other) const;
};
