#pragma once
#include <cstddef>
#include <format>
#include <iostream>
#include <stdexcept>
#include <variant>

template <class... Ts>
struct overloaded : Ts... {
    using Ts::operator()...;
};
template <class... Ts>
overloaded(Ts...) -> overloaded<Ts...>;

struct Value {
    Value(double value)
        : as { value }
    {
    }
    Value(bool value)
        : as { value }
    {
    }
    Value(nullptr_t null)
        : as { null }
    {
    }

    std::variant<double, bool, nullptr_t> as;

    void print() const
    {
        std::visit(overloaded {
                       [](const double d) { std::cout << std::format("{}", d); },
                       [](const bool b) { std::cout << (b ? "true" : "false"); },
                       [](nullptr_t) { std::cout << "NULL"; } },
            as);
    }
    Value operator+(const Value& other) const
    {
        if (std::holds_alternative<double>(as) && std::holds_alternative<double>(other.as)) {
            return Value(std::get<double>(as) + std::get<double>(other.as));
        }
        throw std::runtime_error("Invalid operation: can only add numbers");
    }

    Value operator!() const
    {
        return std::visit(overloaded {
                              [](auto) { return Value { false }; },
                              [](nullptr_t null) { return Value { true }; },
                              [](bool b) { return Value { !b }; },
                          },
            as);
    }

    Value operator-(const Value& other) const
    {
        if (std::holds_alternative<double>(as) && std::holds_alternative<double>(other.as)) {
            return Value(std::get<double>(as) - std::get<double>(other.as));
        }
        throw std::runtime_error("Invalid operation: can only subtract numbers");
    }
    Value operator==(const Value& other) const
    {
        return std::visit(overloaded {
                              [](const double a, const double b) { return Value(a == b); },
                              [](const bool a, const bool b) { return Value(a == b); },
                              [](nullptr_t, nullptr_t) { return Value(true); },
                              [](const auto&, const auto&) { return Value(false); } },
            as, other.as);
    }

    Value operator>(const Value& other) const
    {
        return std::visit(overloaded {
                              [](const double a, const double b) { return Value(a > b); },
                              [](const auto&, const auto&) {
                                  throw std::runtime_error("Invalid operation: can only compare numbers");
                                  // unreachable
                                  return Value { nullptr };
                              } },
            as, other.as);
    }

    Value operator<(const Value& other) const
    {
        return std::visit(overloaded {
                              [](const double a, const double b) { return Value(a < b); },
                              [](const auto&, const auto&) {
                                  throw std::runtime_error("Invalid operation: can only compare numbers");
                                  // unreachable
                                  return Value { nullptr };
                              } },
            as, other.as);
    }

    Value operator!=(const Value& other) const
    {
        return Value(!std::get<bool>((*this == other).as));
    }

    Value operator<=(const Value& other) const
    {
        return Value(!std::get<bool>((*this > other).as));
    }

    Value operator>=(const Value& other) const
    {
        return Value(!std::get<bool>((*this < other).as));
    }
    Value operator*(const Value& other) const
    {
        if (std::holds_alternative<double>(as) && std::holds_alternative<double>(other.as)) {
            return Value(std::get<double>(as) * std::get<double>(other.as));
        }
        throw std::runtime_error("Invalid operation: can only multiply numbers");
    }

    Value operator/(const Value& other) const
    {
        if (std::holds_alternative<double>(as) && std::holds_alternative<double>(other.as)) {
            if (std::get<double>(other.as) == 0) {
                throw std::runtime_error("Division by zero");
            }
            return Value(std::get<double>(as) / std::get<double>(other.as));
        }
        throw std::runtime_error("Invalid operation: can only divide numbers");
    }

    Value operator-() const
    {
        if (std::holds_alternative<double>(as)) {
            return Value(-std::get<double>(as));
        }
        throw std::runtime_error("Invalid operation: can only negate numbers");
    }

    Value& operator+=(const Value& other)
    {
        *this = *this + other;
        return *this;
    }

    Value& operator-=(const Value& other)
    {
        *this = *this - other;
        return *this;
    }

    Value& operator*=(const Value& other)
    {
        *this = *this * other;
        return *this;
    }

    Value& operator/=(const Value& other)
    {
        *this = *this / other;
        return *this;
    }
};
