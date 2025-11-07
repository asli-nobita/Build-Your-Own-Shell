#include <iostream>
#include <string>
#include <sstream>

int main() {
    // Flush after every std::cout / std:cerr
    std::cout << std::unitbuf;
    std::cerr << std::unitbuf;

    std::string exit_msg = "exit 0";

    while (1) {
        std::cout << "$ ";
        std::string input;
        std::getline(std::cin, input);
        if (input == exit_msg) {
            // std::cout << "Exiting...\n";  
            std::exit(0);
        }
        // check if the first word is "echo" 
        std::istringstream iss(input);
        std::string first;
        iss >> first;
        if (first == "echo") {
            std::string rest;
            std::getline(iss, rest);
            auto pos = rest.find_first_not_of(' ');
            if (pos != std::string::npos) {
                rest.erase(0, pos);
            }
            else {
                rest.clear();
            }
            std::cout << rest << "\n";
            continue;
        }
        else std::cout << input << ": command not found\n";
    }
}
