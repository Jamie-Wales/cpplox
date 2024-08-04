#include "Scanner.h"
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
    for (auto& token : tokens) {
        std::cout << token.lexeme << std::endl;
    }
}

void repl()
{
    std::string line;
    while (true) {
        std::cout << "> ";
        if (!std::getline(std::cin, line)) {
            std::cout << '\n';
            break;
        }

        std::cout << line << std::endl;
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
