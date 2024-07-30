#include "Chunk.h"
#include "vMachine.h"

vMachine jamievm;

int main(int argv, char* argc[])
{
    auto chunk = Chunk { 100 };
    for (int i = 0; i < 300; i++) {
        chunk.writeConstant(1000 + i, 0 + i);
    }
    chunk.disassembleChunk();
};
