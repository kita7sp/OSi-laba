#include <iostream>
#include <cstdlib>
#include <string>
#include <sstream>

int main() {
    setenv("PROC_TO_KILL", "Calculator,TextEdit", 1);

    /*std::cout << "\nBefore - Calculator processes:\n";
    system("pgrep Calculator");
    system("./killer --name Calculator");
    std::cout << "After - Calculator processes:\n";
    system("pgrep Calculator");*/

    std::cout << "\nGet ID of TextEdit:\n";
    system("pgrep TextEdit > /tmp/pid.txt");
    system("if [ -s /tmp/pid.txt ]; then head -1 /tmp/pid.txt | xargs ./killer --id; fi");

    std::cout << "\nTest PROC_TO_KILL\n";
    char* env = std::getenv("PROC_TO_KILL");
    system("./killer --name DontMetter");

    unsetenv("PROC_TO_KILL");
    std::cout << "\nPROC_TO_KILL deleted\n";

    return 0;
}

/* g++ -o killer killer.cpp
g++ -o user user.cpp
./user */