#pragma once
#include "Object.h"
#include <cstddef>
#include <string>
#include <variant>

struct Value {
    std::variant<double, bool, nullptr_t, Obj*> as;

    // Default constructor
    Value()
        : as(nullptr)
    {
    }

    // Parameterized constructors
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

    // Destructor
    ~Value()
    {
        if (auto obj_ptr = std::get_if<Obj*>(&as)) {
            delete *obj_ptr;
        }
    }

    // Copy constructor
    Value(const Value& other)
        : as(other.as)
    {
        if (auto obj_ptr = std::get_if<Obj*>(&as)) {
            as = new Obj(**obj_ptr);
        }
    }

    // Move constructor
    Value(Value&& other) noexcept
        : as(std::move(other.as))
    {
        if (auto obj_ptr = std::get_if<Obj*>(&other.as)) {
            *obj_ptr = nullptr;
        }
    }

    // Copy assignment operator
    Value& operator=(const Value& other);

    // Move assignment operator
    Value& operator=(Value&& other) noexcept;

    // Other member functions
    std::string to_string();
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
