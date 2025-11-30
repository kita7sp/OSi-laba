#include <iostream>
#include <sstream>
#include <string>

int main() {
    std::string input;
    std::getline(std::cin, input);
    std::istringstream iss(input);
    int number;
    
    while (iss >> number) {
        std::cout << number * number * number << " ";
    }
    std::cout << std::endl;
    return 0;
}