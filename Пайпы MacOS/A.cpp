#include <iostream>
#include <sstream>
#include <string>

const int N = 1;

int main() {
    std::string input;
    std::getline(std::cin, input);
    std::istringstream iss(input);
    int number;
    
    while (iss >> number) {
        std::cout << number + N << " ";
    }
    std::cout << std::endl;
    return 0;
}