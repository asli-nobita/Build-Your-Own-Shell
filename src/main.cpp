#include "helper.h"

int main() {
    // Flush after every std::cout / std:cerr
    std::cout << std::unitbuf;
    std::cerr << std::unitbuf;

    std::unordered_set<std::string> builtin_cmds{ "type","echo","exit","pwd","cd" };

    const char* path_env = std::getenv("PATH");
    if (path_env == nullptr) {
        std::cerr << "Path environment variable does not exist" << std::endl;
        exit(1);
    }
    std::string PATH(path_env);


    while (1) {
        std::cout << "$ ";
        std::string input;
        std::getline(std::cin, input);
        // trim(input);

        try {
            auto [parsed_cmd, is_redirect] = parse_command(input);
            auto command = parsed_cmd[0];
            std::string redirect_filename;
            if (is_redirect) {
                redirect_filename = parsed_cmd.back();
                // std::cout << "Debugging: Parsed filename as " << redirect_filename << std::endl;
            }
            // a span is a view into a container, it does not create a copy. here we just need the array minus the first element as read only 
            auto args = std::span<std::string>(parsed_cmd.begin() + 1, (is_redirect ? parsed_cmd.end() - 1 : parsed_cmd.end()));
            // std::cout << "Debugging: args size is " << args.size() << std::endl;

            std::ostringstream output;

            if (command == "type") {
                // command as argument should be single word
                if (args.empty()) std::cerr << ": not found\n";
                else if (args.size() > 1) std::cout << "Invalid argument" << std::endl;
                // else if (args.find(' ') != std::string::npos) std::cout << args << ": not found\n";
                else {
                    auto argv = args[0];
                    if (builtin_cmds.count(argv)) output << argv << " is a shell builtin\n";
                    else {
                        const char* path_env = std::getenv("PATH");
                        if (path_env != nullptr) {
                            std::string PATH(path_env);
                            auto exe_path = search_in_path(PATH, argv);
                            if (exe_path.empty()) {
                                std::cerr << argv << ": not found" << std::endl;
                            }
                            else {
                                output << argv << " is " << exe_path << "\n";
                            }
                        }
                    }
                }
            }
            else if (command == "echo") {
                // concatenate all arguments and print to output stream
                for (auto& arg : args) {
                    output << arg << " ";
                }
                output << "\n";
            }
            else if (command == "exit") {
                std::exit(0);
            }
            else if (command == "pwd") {
                try {
                    std::filesystem::path current_dir = std::filesystem::current_path();
                    output << current_dir.string() << "\n";
                }
                catch (const std::filesystem::filesystem_error& e) {
                    std::cerr << e.what() << std::endl;
                }
            }
            else if (command == "cd") {
                if (args.size() > 1) {
                    std::cerr << "Invalid argument" << std::endl;
                    std::exit(1);
                }
                auto argv = args[0];
                const char* home_env = std::getenv("HOME");
                const std::string target_dir = (argv == "~") ? std::string(home_env) : argv;
                if (std::filesystem::exists(target_dir)) {
                    std::filesystem::current_path(target_dir);
                }
                else {
                    std::cerr << command << ": " << target_dir << ": No such file or directory" << std::endl;
                }
            }
            else {
                // search for executable command in PATH 
                auto exe_path = search_in_path(PATH, command);
                if (!exe_path.empty()) {
                    pid_t pid = fork();
                    if (pid == 0) {
                        // child process 
                        // execv expects a argument list of char pointers 
                        // .c_str() returns const pointers, so we need a cast
                        std::vector<char*> parsed_args_ptrs;
                        // first argument should be name of executable 
                        parsed_args_ptrs.push_back(const_cast<char*>(command.c_str()));
                        for (auto& arg : args) parsed_args_ptrs.push_back(const_cast<char*>(arg.c_str()));
                        parsed_args_ptrs.push_back(nullptr);

                        if (is_redirect) {
                            int fd = open(redirect_filename.c_str(), O_WRONLY | O_TRUNC | O_CREAT, 0777);
                            dup2(fd, STDOUT_FILENO);
                            close(fd);
                        }
                        execv(exe_path.c_str(), parsed_args_ptrs.data());
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
                    std::cerr << command << ": not found" << std::endl;
                }
            }

            if (!output.str().empty()) {
                if (is_redirect) {
                    // open or create file in write mode and write output to file 
                    int fd = open(redirect_filename.c_str(), O_WRONLY | O_TRUNC | O_CREAT, 0777);
                    std::string buf = output.str();
                    size_t NBYTES = buf.length();
                    auto bytes = write(fd, buf.c_str(), NBYTES);
                    if (bytes == -1) {
                        std::cerr << "Error writing to file\n";
                        continue;
                    }
                    // std::cout << "Debugging: " << bytes << " bytes written to file" << std::endl;
                    close(fd);
                }
                else {
                    // write to stdout 
                    std::cout << output.str();
                }
            }
        }
        catch (std::invalid_argument& e) {
            std::cerr << e.what() << std::endl;
            continue;
        }


    }
}
