#pragma once
#include "Object.h"
#include "Stringinterner.h"
#include "Visit.h"
#include <cstddef>
#include <format>
#include <iostream>
#include <stdexcept>
#include <string>
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
    Value& operator=(const Value& other)
    {
        if (this != &other) {
            if (auto obj_ptr = std::get_if<Obj*>(&as)) {
                delete *obj_ptr;
            }
            as = other.as;
            if (auto obj_ptr = std::get_if<Obj*>(&as)) {
                as = new Obj(**obj_ptr);
            }
        }
        return *this;
    }

    Value& operator=(Value&& other) noexcept
    {
        if (this != &other) {
            if (auto obj_ptr = std::get_if<Obj*>(&as)) {
                delete *obj_ptr;
            }
            as = std::move(other.as);
            if (auto obj_ptr = std::get_if<Obj*>(&other.as)) {
                *obj_ptr = nullptr;
            }
        }
        return *this;
    }

    void print() const
    {
        std::visit(overloaded {
                       [](double d) { std::cout << std::format("{}", d); },
                       [](bool b) { std::cout << (b ? "true" : "false"); },
                       [](nullptr_t) { std::cout << "nil"; },
                       [](Obj* obj) { obj->print(); } },
            as);
    }

    Value operator+(const Value& other) const
    {
        return std::visit(overloaded {
                              [](double a, double b) {
                                  return Value(a + b);
                              },
                              [](Obj* a, Obj* b) -> Value {
                                  if (auto sa = std::get_if<ObjString>(&a->as)) {
                                      if (auto sb = std::get_if<ObjString>(&b->as)) {
                                          auto& interner = StringInterner::instance();
                                          if (auto existing = interner.find(*sa->str + *sb->str)) {
                                              return Value(new Obj(ObjString(existing)));
                                          }
                                          const std::string* interned = interner.intern(*sa->str + *sb->str);
                                          return Value(new Obj(ObjString(interned)));
                                      }
                                  }
                                  throw std::runtime_error("Can only concatenate string objects");
                              },
                              [](Obj* a, double b) -> Value {
                                  if (auto sa = std::get_if<ObjString>(&a->as)) {
                                      auto& interner = StringInterner::instance();
                                      std::string numStr = std::format("{:.6g}", b);
                                      if (auto existing = interner.find(*sa->str + numStr)) {
                                          return Value(new Obj(ObjString(existing)));
                                      }
                                      const std::string* interned = interner.intern(*sa->str + numStr);
                                      return Value(new Obj(ObjString(interned)));
                                  }
                                  throw std::runtime_error("Can only concatenate string with number");
                              },
                              [](double a, Obj* b) -> Value {
                                  if (auto sb = std::get_if<ObjString>(&b->as)) {
                                      auto& interner = StringInterner::instance();
                                      std::string numStr = std::format("{:.6g}", a);
                                      if (auto existing = interner.find(numStr + *sb->str)) {
                                          return Value(new Obj(ObjString(existing)));
                                      }
                                      const std::string* interned = interner.intern(numStr + *sb->str);
                                      return Value(new Obj(ObjString(interned)));
                                  }
                                  throw std::runtime_error("Can only concatenate number with string");
                              },
                              [](const auto&, const auto&) -> Value {
                                  throw std::runtime_error("Invalid operation: can only add numbers or concatenate strings");
                              } },
            as, other.as);
    }
    Value operator==(const Value& other) const
    {
        return std::visit(overloaded {
                              [](double a, double b) { return Value(a == b); },
                              [](bool a, bool b) { return Value(a == b); },
                              [](nullptr_t, nullptr_t) { return Value(true); },
                              [](Obj* a, Obj* b) {
                                  return std::visit(overloaded {
                                                        [](const ObjString& sa, const ObjString& sb) {
                                                            return Value(sa.str == sb.str);
                                                        },
                                                        [](const auto&, const auto&) { return Value(false); } },
                                      a->as, b->as);
                              },
                              [](const auto&, const auto&) { return Value(false); } },
            as, other.as);
    }

    Value& operator+=(const Value& other)
    {
        *this = *this + other;
        return *this;
    }

    Value& operator*=(const Value& other)
    {
        as = std::visit(overloaded {
                            [](double a, double b) { return Value(a * b); },
                            [](const auto&, const auto&) -> Value {
                                throw std::runtime_error("Invalid operation: can only multiply numbers");
                            } },
            as, other.as)
                 .as;
        return *this;
    }

    Value& operator/=(const Value& other)
    {
        as = std::visit(overloaded {
                            [](double a, double b) {
                                if (b == 0)
                                    throw std::runtime_error("Division by zero");
                                return Value(a / b);
                            },
                            [](const auto&, const auto&) -> Value {
                                throw std::runtime_error("Invalid operation: can only divide numbers");
                            } },
            as, other.as)
                 .as;
        return *this;
    }

    Value operator-() const
    {
        return std::visit(overloaded {
                              [](double a) { return Value(-a); },
                              [](const auto&) -> Value {
                                  throw std::runtime_error("Invalid operation: can only negate numbers");
                              } },
            as);
    }

    Value operator!() const
    {
        return std::visit(overloaded {
                              [](double) { return Value(false); },
                              [](bool b) { return Value(!b); },
                              [](nullptr_t) { return Value(true); },
                              [](Obj*) { return Value(false); } },
            as);
    }
    Value operator-(const Value& other) const
    {
        return std::visit(overloaded {
                              [](double a, double b) { return Value(a - b); },
                              [](const auto&, const auto&) -> Value {
                                  throw std::runtime_error("Invalid operation: can only subtract numbers");
                              } },
            as, other.as);
    }

    Value operator>(const Value& other) const
    {
        return std::visit(overloaded {
                              [](double a, double b) { return Value(a > b); },
                              [](const auto&, const auto&) -> Value {
                                  throw std::runtime_error("Invalid operation: can only compare numbers");
                              } },
            as, other.as);
    }
};
