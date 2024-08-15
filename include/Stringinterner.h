#pragma once
#include "unordered_set"
class StringInterner {
private:
    std::unordered_set<std::string> pool;

public:
    const std::string* intern(const std::string& s);
    [[nodiscard]] const std::string* find(const std::string& s) const;
    static StringInterner& instance()
    {
        static StringInterner interner;
        return interner;
    }
};
