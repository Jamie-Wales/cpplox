#include "Object.h"
#include "Visit.h"
#include <format>
#include <iostream>

void Obj::print() const
{
    std::cout << to_string();
}

std::string Obj::to_string() const
{
    return std::visit(overloaded {
                          [](const ObjString& s) -> std::string {
                              return *s.str;
                          },
                          [](const ObjFunction& f) -> std::string {
                              return std::format("<function {}>", f.name);
                          },
                          [](const ObjInstance&) -> std::string {
                              return "<instance>";
                          },
                          [](const ObjNative&) -> std::string {
                              return "<native fn>";
                          },
                          [](const ObjClosure& c) -> std::string {
                              return std::format("<closure {}>", c.pFunction->name);
                          },
                          [](const ObjUpvalue& u) -> std::string {
                              return std::format("<up value {}>", u.location->to_string());
                          } },
        as);
}
