#include "helper.h"

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
        std::string command = get_command(input); 
        std::string args = get_command_arguments(input); 
        std::cout << "Command: " << command << std::endl;  
        std::cout << "Args: " << args << std::endl; 
        if(command == "type") { 
            // command as argument should be single word
            trim(args);
            if(args.empty()) std::cout << ": command not found\n"; 
            else if(args.find(' ') != std::string::npos) std::cout << args << ": command not found\n"; 
            else { 
                if(args == "type" || args == "echo" || args == "exit") std::cout << args << " is a shell builtin\n";
                else std::cout << args << ": command not found\n"; 
            }
        } 
        else if(command == "echo") { 
            std::cout << args << "\n"; 
        }
        else std::cout << input << ": command not found\n";
    }
}
