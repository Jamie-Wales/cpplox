#include "run.h"
#include "Object.h"
#include "Parser.h"
#include "Printer.h"
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
    Parser parser { tokens };
    auto program = parser.parseProgram();
    Printer pr {};
    for (auto& st : program) {
        pr.print(*st);
    }
    // Compiler compiler { tokens };
    // if (std::optional<ObjFunction*> main = compiler.compile()) {
    //     vm.load(*main);
    //     vm.run();
    // } else {
    //     std::cerr << "Compilation failed." << std::endl;
    // }
}

void runRepl()
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
        // TODO: Avoid reusing compiler so const expressions work in REPL
        // Compiler compiler { tokens };
        // if (std::optional<ObjFunction*> main = compiler.compile()) {
        //     vm.load(*main);
        //     vm.execute();
        // } else {
        //     std::cerr << "Compilation failed." << std::endl;
        // }
    }
}
