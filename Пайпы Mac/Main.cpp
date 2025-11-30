#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string>

using namespace std;

// Main -> pInput -> M -> p1 -> A -> p2 -> P -> p3 -> S -> stdout

void doProc(const char* prog, int inputFd, int outputFd) {
    pid_t pid = fork();
    if (pid == 0) {
        if (inputFd != STDIN_FILENO) {
            dup2(inputFd, STDIN_FILENO);
            close(inputFd);
        }
        if (outputFd != STDOUT_FILENO) {
            dup2(outputFd, STDOUT_FILENO);
            close(outputFd);
        }
        execl(prog, prog, nullptr);
        perror("execl failed");
        exit(1);
    }
}

int main() {
    int p1[2], p2[2], p3[2], pInput[2];
    pipe(pInput);
    pipe(p1);
    pipe(p2);
    pipe(p3);

    // S
    doProc("./S", p3[0], STDOUT_FILENO);
    close(p3[0]);

    // P
    doProc("./P", p2[0], p3[1]);
    close(p2[0]);
    close(p3[1]);

    // A
    doProc("./A", p1[0], p2[1]);
    close(p1[0]);
    close(p2[1]);

    // M
    doProc("./M", pInput[0], p1[1]);
    close(pInput[0]);
    close(p1[1]);

    // используемая строка
    string input = "1 2 3 4";
    write(pInput[1], input.c_str(), input.length());
    write(pInput[1], "\n", 1);
    close(pInput[1]);

    cout << "Input: " << input << endl;
    cout << "Result: ";
    cout.flush();

    // пока ввсе дочерние завершатся
    for (int i = 0; i < 4; i++) {
        wait(nullptr);
    }

    return 0;
}

/*
g++ M.cpp -o M
g++ A.cpp -o A
g++ P.cpp -o P
g++ S.cpp -o S
g++ Main.cpp -o Main
./Main
*/