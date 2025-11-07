#include <iostream>
#include <string>

int main() {
    // Flush after every std::cout / std:cerr
    std::cout << std::unitbuf;
    std::cerr << std::unitbuf;

    std::string exit_msg = "exit 0"; 

    while (1) {
        std::cout << "$ ";
        std::string input;
        std::getline(std::cin, input); 
        if(input.compare(exit_msg) == 0) { 
            // std::cout << "Exiting...\n";  
            std::exit(0); 
        }
        std::cout << input << ": command not found\n"; 
    }
}
