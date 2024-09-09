#include "run.h"
#include <iostream>
int main(const int argc, const char* argv[])
{
    if (argc == 1) {
        runRepl();
    } else if (argc == 2) {
        runFile(argv[1]);
    } else {
        std::cout << "Usage vm [script] || vm" << std::endl;
    }
}
