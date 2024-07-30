#pragma once
#include "Chunk.h"
#include "Instructions.h"
enum class vState {
    OK,
    COMPILE_ERROR,
    RUNTIME_ERROR
};

class vMachine {
public:
    vMachine() = default;
    vMachine(vMachine&&) = default;
    vMachine(const vMachine&) = default;
    vMachine& operator=(vMachine&&) = default;
    vMachine& operator=(const vMachine&) = default;
    ~vMachine() = default;

private:
    vState state;
    u_int8_t ip;
    Chunk instructions;
    void run()
    {
        for (auto& byte : instructions.code) {
            switch (byte) {
            case OP_RETURN:
                state = vState::OK;
                break;
            case OP_CONSTANT:
                break;
            }
            ip++;
        }
    }
};
