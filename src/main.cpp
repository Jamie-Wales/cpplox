#include "Compiler.h"
#include "Scanner.h"
#include "vMachine.h"
#include <Tests.h>
#include <fstream>
#include <iostream>
#include <string>

std::string readFile(const std::string& path)
{
    std::ifstream inputFileStream(path);
    std::string parsed_file = { std::istreambuf_iterator<char>(inputFileStream),
        std::istreambuf_iterator<char>() };
    return parsed_file;
}

void runFile(std::string path)
{
    std::string source = readFile(path);
    Scanner scanner { source };
    auto tokens = scanner.tokenize();
    Compiler compiler { tokens };

    std::optional<Chunk> chunk = compiler.compile();
    if (chunk) {
        vMachine vm { *chunk };
        vm.run();
    } else {
        std::cerr << "Compilation failed." << std::endl;
    }
}

void repl()
{
    std::string line;
    vMachine vm;
    while (true) {
        std::cout << "> ";
        if (!std::getline(std::cin, line)) {
            std::cout << '\n';
            break;
        }
        if (line == "exit") {
            std::cout << "Goodbye!" << std::endl;
            break;
        }
        Scanner scanner { line };
        auto tokens = scanner.tokenize();
        scanner.addEOFToken();
        Compiler compiler { tokens };
        std::optional<Chunk> chunk = compiler.compile();
        if (chunk) {
            vm.execute(*chunk);  // Use execute instead ld of run
        } else {
            std::cerr << "Compilation failed." << std::endl;
        }
    }
}
int main(int argc, char* argv[])
{
    test();
    if (argc == 1) {
        repl();
    } else if (argc == 2) {
        runFile(argv[1]);
    } else {
        std::cout << "Usage vm [script] || vm" << std::endl;
    }
    return 0;
}
