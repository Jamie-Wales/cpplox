#include "Object.h"
#include <format>
#include <iostream>

void Obj::print() const
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
