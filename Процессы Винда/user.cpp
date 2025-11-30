#include <iostream>
#include <cstdlib>
#include <string>

int main() {
    putenv("PROC_TO_KILL=Calculator.exe,notepad.exe");

    std::cout << "\nBefore - Calculator processes:\n";
    system("tasklist | findstr Calculator.exe");
    system("killer.exe --name Calculator.exe");
    std::cout << "After - Calculator processes:\n";
    system("tasklist | findstr Calculator.exe");

    std::cout << "\nGet ID of notepad.exe:\n";
    system("tasklist | findstr notepad.exe > %TEMP%\\pid.txt");

    system(
        "for /f \"tokens=2\" %i in (%TEMP%\\pid.txt) do killer.exe --id %i"
    );

    std::cout << "\nTest PROC_TO_KILL\n";
    char* env = std::getenv("PROC_TO_KILL");
    system("killer.exe --name DontMatter");

    putenv("PROC_TO_KILL=");

    std::cout << "\nPROC_TO_KILL deleted\n";

    return 0;
}
