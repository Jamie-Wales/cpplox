#include "Value.h"
#include "Object.h"
#include "Stringinterner.h"
#include "Visit.h"
#include <cstddef>
#include <format>
#include <iostream>
#include <stdexcept>
#include <string>
#include <variant>

void Value::print() const
{
    std::visit(overloaded {
                   [](double d) -> void { std::cout << std::format("{}", d); },
                   [](bool b) -> void { std::cout << (b ? "true" : "false"); },
                   [](nullptr_t) -> void { std::cout << "nil"; },
                   [](Obj* obj) -> void { obj->print(); } },
        as);
}

std::string Value::to_string() const
{
    return std::visit(overloaded {
                          [](double d) -> std::string { return std::format("{}", d); },
                          [](const bool b) -> std::string { return (b ? "true" : "false"); },
                          [](nullptr_t) -> std::string { return "nil"; },
                          [](const Obj* o) -> std::string {
                              return o->to_string();
                          },
                      },
        as);
}

Value& Value::operator+=(const Value& other)
{
    *this = *this + other;
    return *this;
}

Value Value::operator<(const Value& other) const
{
    return std::visit(overloaded {
                          [](double a, double b) -> Value { return { a < b }; },
                          [](const auto&, const auto&) -> Value {
                              throw std::runtime_error("Invalid operation: can only compare numbers");
                          } },
        as, other.as);
}

Value Value::operator>=(const Value& other) const
{
    return std::visit(overloaded {
                          [](double a, double b) -> Value { return { a >= b }; },
                          [](const auto&, const auto&) -> Value {
                              throw std::runtime_error("Invalid operation: can only compare numbers");
                          } },
        as, other.as);
}

bool Value::isTruthy() const
{
    return std::visit(overloaded {
                          [](double d) { return d != 0.0; },
                          [](bool b) { return b; },
                          [](nullptr_t) { return false; },
                          [](Obj* obj) {
                              if (auto str = std::get_if<ObjString>(&obj->as)) {
                                  return !str->str->empty();
                              }
                              return true;
                          } },
        as);
}

Value Value::operator+(const Value& other) const
{
    return std::visit(overloaded {
                          [](double a, double b) -> Value {
                              return { a + b };
                          },
                          [](Obj* a, Obj* b) -> Value {
                              if (auto sa = std::get_if<ObjString>(&a->as)) {
                                  if (auto sb = std::get_if<ObjString>(&b->as)) {
                                      auto& interner = StringInterner::instance();
                                      if (auto existing = interner.find(*sa->str + *sb->str)) {
                                          return { new Obj(ObjString(existing)) };
                                      }
                                      const std::string* interned = interner.intern(*sa->str + *sb->str);
                                      return { new Obj(ObjString(interned)) };
                                  }
                              }
                              throw std::runtime_error("Can only concatenate string objects");
                          },
                          [](Obj* a, double b) -> Value {
                              if (const auto sa = std::get_if<ObjString>(&a->as)) {
                                  auto& interner = StringInterner::instance();
                                  const std::string numStr = std::format("{:.6g}", b);
                                  if (const auto existing = interner.find(*sa->str + numStr)) {
                                      return { new Obj(ObjString(existing)) };
                                  }
                                  const std::string* interned = interner.intern(*sa->str + numStr);
                                  return { new Obj(ObjString(interned)) };
                              }
                              throw std::runtime_error("Can only concatenate string with number");
                          },
                          [](double a, Obj* b) -> Value {
                              if (auto sb = std::get_if<ObjString>(&b->as)) {
                                  auto& interner = StringInterner::instance();
                                  std::string numStr = std::format("{:.6g}", a);
                                  if (auto existing = interner.find(numStr + *sb->str)) {
                                      return { new Obj(ObjString(existing)) };
                                  }
                                  const std::string* interned = interner.intern(numStr + *sb->str);
                                  return { new Obj(ObjString(interned)) };
                              }
                              throw std::runtime_error("Can only concatenate number with string");
                          },
                          [](const auto&, const auto&) -> Value {
                              throw std::runtime_error("Invalid operation: can only add numbers or concatenate strings");
                          } },
        as, other.as);
}

Value Value::operator==(const Value& other) const
{
    return std::visit(overloaded {
                          [](double a, double b) -> Value { return { a == b }; },
                          [](bool a, bool b) -> Value { return { a == b }; },
                          [](nullptr_t, nullptr_t) -> Value { return { true }; },
                          [](Obj* a, Obj* b) -> Value {
                              return std::visit(overloaded {
                                                    [](const ObjString& sa, const ObjString& sb) -> Value {
                                                        return { sa.str == sb.str };
                                                    },
                                                    [](const auto&, const auto&) -> Value { return { false }; } },
                                  a->as, b->as);
                          },
                          [](const auto&, const auto&) -> Value { return { false }; } },
        as, other.as);
}

Value& Value::operator/=(const Value& other)
{
    as = std::visit(overloaded {
                        [](double a, double b) -> Value {
                            if (b == 0)
                                throw std::runtime_error("Division by zero");
                            return { a / b };
                        },
                        [](const auto&, const auto&) -> Value {
                            throw std::runtime_error("Invalid operation: can only divide numbers");
                        } },
        as, other.as)
             .as;
    return *this;
}

Value Value::operator-() const
{
    return std::visit(overloaded {
                          [](double a) -> Value { return { -a }; },
                          [](const auto&) -> Value {
                              throw std::runtime_error("Invalid operation: can only negate numbers");
                          } },
        as);
}

Value Value::operator!() const
{
    return std::visit(overloaded {
                          [](double) { return Value(false); },
                          [](bool b) { return Value(!b); },
                          [](nullptr_t) { return Value(true); },
                          [](Obj*) { return Value(false); } },
        as);
}

Value Value::operator-(const Value& other) const
{
    return std::visit(overloaded {
                          [](double a, double b) -> Value { return { a - b }; },
                          [](const auto&, const auto&) -> Value {
                              throw std::runtime_error("Invalid operation: can only subtract numbers");
                          } },
        as, other.as);
}

Value Value::operator>(const Value& other) const
{
    return std::visit(overloaded {
                          [](double a, double b) -> Value { return { a > b }; },
                          [](const auto&, const auto&) -> Value {
                              throw std::runtime_error("Invalid operation: can only compare numbers");
                          } },
        as, other.as);
};

bool Value::isString() const
{
    return std::visit(overloaded {
                          [](Obj* obj) { return std::holds_alternative<ObjString>(obj->as); },
                          [](const auto&) { return false; } },
        as);
}

bool Value::isNumber() const
{
    return std::visit(overloaded {
                          [](double) { return true; },
                          [](const auto&) { return false; } },
        as);
}

double Value::asNumber() const
{
    return std::visit(overloaded {
                          [](double d) { return d; },
                          [](const auto&) -> double { throw std::runtime_error("Value is not a number"); } },
        as);
}

Value Value::operator*(const Value& other) const
{
    return std::visit(overloaded {
                          [](double a, double b) -> Value { return { a * b }; },
                          [](Obj* a, double b) -> Value {
                              if (auto sa = std::get_if<ObjString>(&a->as)) {
                                  int repeat = static_cast<int>(b);
                                  if (repeat < 0) {
                                      throw std::runtime_error("Cannot multiply string by negative number");
                                  }
                                  std::string result;
                                  result.reserve(sa->str->length() * repeat);
                                  for (int i = 0; i < repeat; ++i) {
                                      result += *sa->str;
                                  }
                                  auto& interner = StringInterner::instance();
                                  const std::string* interned = interner.intern(result);
                                  return { new Obj(ObjString(interned)) };
                              }
                              throw std::runtime_error("Can only multiply string by number");
                          },
                          [](double a, Obj* b) -> Value {
                              if (auto sb = std::get_if<ObjString>(&b->as)) {
                                  int repeat = static_cast<int>(a);
                                  if (repeat < 0) {
                                      throw std::runtime_error("Cannot multiply string by negative number");
                                  }
                                  std::string result;
                                  result.reserve(sb->str->length() * repeat);
                                  for (int i = 0; i < repeat; ++i) {
                                      result += *sb->str;
                                  }
                                  auto& interner = StringInterner::instance();
                                  const std::string* interned = interner.intern(result);
                                  return { new Obj(ObjString(interned)) };
                              }
                              throw std::runtime_error("Can only multiply string by number");
                          },
                          [](const auto&, const auto&) -> Value {
                              throw std::runtime_error("Invalid operation: can only multiply numbers or string by number");
                          } },
        as, other.as);
}

Value Value::operator*=(const Value& other)
{
    *this = *this * other;
    return *this;
}
