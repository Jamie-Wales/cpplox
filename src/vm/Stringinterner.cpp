
#include "Stringinterner.h"

const std::string* StringInterner::find(const std::string& s) const
{
    const auto it = pool.find(s);
    return it != pool.end() ? &(*it) : nullptr;
}

const std::string* StringInterner::intern(const std::string& s)
{
    auto [it, inserted] = pool.insert(s);
    return &(*it);
}
