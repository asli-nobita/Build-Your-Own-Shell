#include "helper.h"

int main() {
    // Flush after every std::cout / std:cerr
    std::cout << std::unitbuf;
    std::cerr << std::unitbuf;

    // std::string exit_msg = "exit 0";

    while (1) {
        std::cout << "$ ";
        std::string input;
        std::getline(std::cin, input);

        std::string command = get_command(input); 
        std::string args = get_command_arguments(input); 
 
        if(command == "type") { 
            // command as argument should be single word
            trim(args);
            if(args.empty()) std::cout << ": not found\n"; 
            else if(args.find(' ') != std::string::npos) std::cout << args << ": not found\n"; 
            else { 
                if(args == "type" || args == "echo" || args == "exit") std::cout << args << " is a shell builtin\n";
                else {  
                    const char* path_env = std::getenv("PATH"); 
                    if(path_env != nullptr) {  
                        std::string PATH(path_env); 
                        std::cout << "Debugging: $PATH=" << PATH << std::endl; 
                        search_in_path(PATH, args); 
                    }
                }
            }
        } 
        else if(command == "echo") { 
            std::cout << args << "\n"; 
        } 
        else if(command == "exit") { 
            std::exit(0);
        }
        else std::cout << input << ": not found\n";
    }
}
