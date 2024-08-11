#pragma once
#include "unordered_set"
class StringInterner {
private:
    std::unordered_set<std::string> pool;

public:
    const std::string* intern(const std::string& s) {
        auto [it, inserted] = pool.insert(s);
        return &(*it);
    }

    const std::string* find(const std::string& s) const {
        auto it = pool.find(s);
        return it != pool.end() ? &(*it) : nullptr;
    }

    static StringInterner& instance() {
        static StringInterner interner;
        return interner;
    }
};
