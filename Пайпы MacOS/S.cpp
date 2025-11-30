#include <iostream>
#include <sstream>
#include <string>

int main() {
    std::string input;
    std::getline(std::cin, input);
    std::istringstream iss(input);
    int number, sum = 0;
    
    while (iss >> number) {
        sum += number;
    }
    std::cout << sum << std::endl;
    return 0;
}