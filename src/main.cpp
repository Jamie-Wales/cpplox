#include "Instructions.h"
#include "vMachine.h"
#include <iostream>

void runTest(const std::string& testName, Chunk& chunk, double expectedResult)
{
    std::cout << "Running test: " << testName << std::endl;
    vMachine vm(chunk);
    vm.run();
    std::cout << "Test " << testName << " " << expectedResult << std::endl;
    std::cout << "------------------------" << std::endl;
}
int main()
{
    {
        Chunk chunk(100);
        chunk.writeConstant(1.0, 1);
        chunk.writeConstant(2.0, 1);
        chunk.writeChunk(OP_CODE::ADD, 1);
        chunk.writeChunk(OP_CODE::RETURN, 1);
        runTest("Simple Addition", chunk, 3.0);
    }

    {
        Chunk chunk(100);
        chunk.writeConstant(5.0, 1);
        chunk.writeConstant(3.0, 1);
        chunk.writeChunk(OP_CODE::SUB, 1);
        chunk.writeChunk(OP_CODE::RETURN, 1);
        runTest("Subtraction", chunk, 2.0);
    }

    {
        Chunk chunk(100);
        chunk.writeConstant(4.0, 1);
        chunk.writeConstant(3.0, 1);
        chunk.writeChunk(OP_CODE::MULT, 1);
        chunk.writeChunk(OP_CODE::RETURN, 1);
        runTest("Multiplication", chunk, 12.0);
    }

    {
        Chunk chunk(100);
        chunk.writeConstant(10.0, 1);
        chunk.writeConstant(2.0, 1);
        chunk.writeChunk(OP_CODE::DIV, 1);
        chunk.writeChunk(OP_CODE::RETURN, 1);
        runTest("Division", chunk, 5.0);
    }

    {
        Chunk chunk(100);
        chunk.writeConstant(5.0, 1);
        chunk.writeChunk(OP_CODE::NEG, 1);
        chunk.writeChunk(OP_CODE::RETURN, 1);
        runTest("Negation", chunk, -5.0);
    }

    {
        Chunk chunk(100);
        chunk.writeConstant(3.0, 1);
        chunk.writeConstant(4.0, 1);
        chunk.writeChunk(OP_CODE::ADD, 1);
        chunk.writeConstant(5.0, 2);
        chunk.writeConstant(2.0, 2);
        chunk.writeChunk(OP_CODE::SUB, 2);
        chunk.writeChunk(OP_CODE::MULT, 3);
        chunk.writeChunk(OP_CODE::RETURN, 3);
        runTest("Complex Expression", chunk, 21.0);
    }

    return 0;
}
