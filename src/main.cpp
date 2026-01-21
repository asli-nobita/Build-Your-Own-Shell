#include "helper.h"

int main() {
    // Flush after every std::cout / std:cerr
    std::cout << std::unitbuf;
    std::cerr << std::unitbuf;

    std::unordered_set<std::string> builtin_cmds{ "type","echo","exit","pwd","cd" };

    while (1) {
        std::cout << "$ ";
        std::string input;
        std::getline(std::cin, input);

        std::string command = get_command(input);
        std::string args = get_command_arguments(input);

        if (command == "type") {
            // command as argument should be single word
            trim(args);
            if (args.empty()) std::cout << ": not found\n";
            // else if (args.find(' ') != std::string::npos) std::cout << args << ": not found\n";
            else {
                if (builtin_cmds.count(args)) std::cout << args << " is a shell builtin\n";
                else {
                    const char* path_env = std::getenv("PATH");
                    if (path_env != nullptr) {
                        std::string PATH(path_env);
                        auto exe_path = search_in_path(PATH, args);
                        if (exe_path.empty()) {
                            std::cout << args << ": not found" << std::endl;
                        }
                        else {
                            std::cout << args << " is " << exe_path << std::endl;
                        }
                    }
                }
            }
        }
        else if (command == "echo") {
            std::cout << args << "\n";
        }
        else if (command == "exit") {
            std::exit(0);
        }
        else if (command == "pwd") {
            try {
                std::filesystem::path current_dir = std::filesystem::current_path();
                std::cout << current_dir.string() << std::endl;
            }
            catch (const std::filesystem::filesystem_error& e) {
                std::cerr << e.what() << std::endl;
            }
        }
        else if (command == "cd") {
            const char* home_env = std::getenv("HOME"); 
            const std::string target_dir = (args == "~") ? std::string(home_env) : args; 
            if (std::filesystem::exists(target_dir)) {
                std::filesystem::current_path(target_dir);
            }
            else {
                std::cerr << command << ": " << target_dir << ": No such file or directory" << std::endl;
            }
        }
        else {
            // search for executable command in PATH 
            const char* path_env = std::getenv("PATH");
            if (path_env != nullptr) {
                std::string PATH(path_env);
                auto exe_path = search_in_path(PATH, command);
                if (!exe_path.empty()) {
                    pid_t pid = fork();
                    if (pid == 0) {
                        // child process 
                        // parse arguments for the executable
                        std::vector<std::string> parsed_args;
                        std::string arg;
                        std::istringstream iss(input);
                        while (std::getline(iss, arg, ' ')) {
                            parsed_args.push_back(arg);
                        }
                        // execv expects a argument list of char pointers 
                        // .c_str() returns const pointers, so we need a cast
                        std::vector<char*> parsed_args_ptrs;
                        for (auto& arg : parsed_args) parsed_args_ptrs.push_back(const_cast<char*>(arg.c_str()));
                        parsed_args_ptrs.push_back(nullptr);

                        // Debugging 
                        // std::cout << exe_path << std::endl;
                        // std::cout << parsed_args_ptrs[0] << std::endl;

                        execv(exe_path.c_str(), parsed_args_ptrs.data());

                        // std::cerr << "Execv failed: " << std::strerror(errno) << std::endl;
                        // std::exit(1);
                    }
                    else if (pid > 0) {
                        int status;
                        waitpid(pid, &status, 0);
                    }
                    else {
                        std::cerr << "Fork failed: " << std::strerror(errno) << std::endl;
                    }
                }
                else {
                    std::cout << command << ": not found" << std::endl;
                }
            }
        }
    }
}
