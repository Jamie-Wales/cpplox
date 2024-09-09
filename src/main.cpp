#include "Compiler.h"
#include "Object.h"
#include "Scanner.h"
#include "vMachine.h"
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

void runFile(const std::string& path)
{

    vMachine vm {};
    std::string source = readFile(path);
    Scanner scanner { source };
    auto tokens = scanner.tokenize();
    Compiler compiler { tokens };
    if (std::optional<ObjFunction*> main = compiler.compile()) {
        vm.load(*main);
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
        // #TOOD don't reuse compiler so const expressions work on repl
        Compiler compiler { tokens };
        if (std::optional<ObjFunction*> main = compiler.compile()) {
            vm.load(*main);
            vm.execute();
        } else {
            std::cerr << "Compilation failed." << std::endl;
        }
    }
}
int main(int argc, char* argv[])
{
    if (argc == 1) {
        repl();
    } else if (argc == 2) {
        runFile(argv[1]);
    } else {
        std::cout << "Usage vm [script] || vm" << std::endl;
    }
    return 0;
}
