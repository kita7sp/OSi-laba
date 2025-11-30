#include <iostream>
#include <cstdlib>
#include <string>
#include <vector>
#include <sstream>

void killFromEnvironment() {
    char* env = std::getenv("PROC_TO_KILL");
    if (env != nullptr) {
        std::string envStr(env);
        std::stringstream ss(envStr);
        std::string processName;
        
        while (std::getline(ss, processName, ',')) {
            std::string command = "taskkill /IM " + processName + " /F";
            system(command.c_str());
            std::cout << "Killed processes from PROC_TO_KILL: " << processName << std::endl;
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "Usage:\n";
        std::cout << "  --id PID\n";
        std::cout << "  --name NAME\n";
        return 1;
    }

    std::string option = argv[1];

    if (option == "--id" && argc == 3) {
        std::string pid = argv[2];
        std::string command = "taskkill /PID " + pid + " /F";
        system(command.c_str());
        std::cout << "Killed process with PID: " << pid << std::endl;

    } else if (option == "--name" && argc == 3) {
        std::string command = "taskkill /IM " + std::string(argv[2]) + " /F";
        system(command.c_str());
        std::cout << "Killed processes with name: " << argv[2] << std::endl;

    } else {
        std::cerr << "Invalid arguments" << std::endl;
        return 1;
    }

    killFromEnvironment();

    return 0;
}
