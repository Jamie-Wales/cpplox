#include "Object.h"
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
                              return std::format("\"{}\"", *s.str);
                          },
                          [](const ObjFunction&) -> std::string {
                              return { "<function>" };
                          },
                          [](const ObjInstance&) -> std::string {
                              return { "<instance>" };
                          } },
        as);
}
