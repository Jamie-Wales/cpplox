#pragma once
#include "Object.h"
#include "Stringinterner.h"
#include "Value.h"
#include <cmath>
#include <iostream>
#include <random>
#include <string>

inline Value getArg(int argCount, Value* args, int index = 0)
{
    if (argCount > index) {
        return args[index];
    }
    return Value(); // Return null for missing arguments
}

inline Value absNative(int argCount, Value* args)
{
    auto v = getArg(argCount, args);
    if (v.isNumber()) {
        return Value(std::abs(v.asNumber()));
    }
    return Value();
}

inline Value powNative(int argCount, Value* args)
{
    auto base = getArg(argCount, args, 0);
    auto exponent = getArg(argCount, args, 1);
    if (base.isNumber() && exponent.isNumber()) {
        return Value(std::pow(base.asNumber(), exponent.asNumber()));
    }
    return Value();
}

inline Value sqrtNative(int argCount, Value* args)
{
    auto v = getArg(argCount, args);
    if (v.isNumber() && v.asNumber() >= 0) {
        return Value(std::sqrt(v.asNumber()));
    }
    return Value();
}

inline Value floorNative(int argCount, Value* args)
{
    auto v = getArg(argCount, args);
    if (v.isNumber()) {
        return Value(std::floor(v.asNumber()));
    }
    return Value();
}

inline Value ceilNative(int argCount, Value* args)
{
    auto v = getArg(argCount, args);
    if (v.isNumber()) {
        return Value(std::ceil(v.asNumber()));
    }
    return Value();
}

inline Value roundNative(int argCount, Value* args)
{
    auto v = getArg(argCount, args);
    if (v.isNumber()) {
        return Value(std::round(v.asNumber()));
    }
    return Value();
}

inline Value randomNative(int argCount, Value* args)
{
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_real_distribution<> dis(0, 1);
    return Value(dis(gen));
}

inline Value isNumberNative(int argCount, Value* args)
{
    auto v = getArg(argCount, args);
    return Value(v.isNumber());
}

inline Value isStringNative(int argCount, Value* args)
{
    auto v = getArg(argCount, args);
    return Value(v.isString());
}

inline Value isNullNative(int argCount, Value* args)
{
    auto v = getArg(argCount, args);
    return Value(std::holds_alternative<nullptr_t>(v.as));
}

inline Value isBoolNative(int argCount, Value* args)
{
    auto v = getArg(argCount, args);
    return Value(std::holds_alternative<bool>(v.as));
}

inline Value toNumberNative(int argCount, Value* args)
{
    auto v = getArg(argCount, args);
    if (v.isNumber()) {
        return v;
    } else if (v.isString()) {
        try {
            return Value(std::stod(v.to_string()));
        } catch (...) {
            // Handle conversion error
        }
    }
    return Value();
}

inline Value toStringNative(int argCount, Value* args)
{
    auto v = getArg(argCount, args);
    const std::string* internedStr = StringInterner::instance().intern(v.to_string());
    return Value(new Obj(ObjString(internedStr)));
}

inline Value toBooleanNative(int argCount, Value* args)
{
    auto v = getArg(argCount, args);
    return Value(v.isTruthy());
}

inline Value printNative(int argCount, Value* args)
{
    for (int i = 0; i < argCount; i++) {
        args[i].print();
        std::cout << " ";
    }
    std::cout << std::endl;
    return Value();
}

inline Value inputNative(int argCount, Value* args)
{
    std::string line;
    std::getline(std::cin, line);
    const std::string* internedStr = StringInterner::instance().intern(line);
    return Value(new Obj(ObjString(internedStr)));
}

inline Value lengthNative(int argCount, Value* args)
{
    auto v = getArg(argCount, args);
    if (v.isString()) {
        return Value(static_cast<double>(v.to_string().length()));
    }
    return Value(0.0);
}

inline Value clockNative(int argCount, Value* args)
{
    if (argCount != 0) {
        return Value(-1.0); // Example error return
    }

    auto now = std::chrono::system_clock::now();
    auto duration = now.time_since_epoch();
    auto nanoseconds = std::chrono::duration_cast<std::chrono::nanoseconds>(duration);

    double seconds = static_cast<double>(nanoseconds.count()) / 1'000'000'000.0;

    return Value(seconds);
}
