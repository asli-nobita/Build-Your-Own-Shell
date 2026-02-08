#include "helper.h"

int main() {
    // Flush after every std::cout / std:cerr
    std::cout << std::unitbuf;
    std::cerr << std::unitbuf;

    std::unordered_set<std::string> builtin_cmds{ "type","echo","exit","pwd","cd" };

    const char* path_env = std::getenv("PATH");
    if (path_env == nullptr) {
        std::cerr << "Path environment variable does not exist" << "\n";
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
            if (is_redirect != redirect_states::NO_REDIRECT) {
                redirect_filename = parsed_cmd.back();
            }
            // a span is a view into a container, it does not create a copy. here we just need the array minus the first element as read only 
            auto args = std::span<std::string>(parsed_cmd.begin() + 1, (is_redirect != redirect_states::NO_REDIRECT ? parsed_cmd.end() - 1 : parsed_cmd.end()));

            std::ostringstream out_stream, error_stream;

            if (command == "type") {
                // command as argument should be single word
                if (args.empty()) error_stream << ": not found\n";
                else if (args.size() > 1) std::cout << "Invalid argument" << "\n";
                else {
                    auto argv = args[0];
                    if (builtin_cmds.count(argv)) out_stream << argv << " is a shell builtin\n";
                    else {
                        const char* path_env = std::getenv("PATH");
                        if (path_env != nullptr) {
                            std::string PATH(path_env);
                            auto exe_path = search_in_path(PATH, argv);
                            if (exe_path.empty()) {
                                error_stream << argv << ": not found" << "\n";
                            }
                            else {
                                out_stream << argv << " is " << exe_path << "\n";
                            }
                        }
                    }
                }
            }
            else if (command == "echo") {
                // concatenate all arguments and print to out_stream 
                for (auto& arg : args) {
                    out_stream << arg << " ";
                }
                out_stream << "\n";
            }
            else if (command == "exit") {
                std::exit(0);
            }
            else if (command == "pwd") {
                try {
                    std::filesystem::path current_dir = std::filesystem::current_path();
                    out_stream << current_dir.string() << "\n";
                }
                catch (const std::filesystem::filesystem_error& e) {
                    error_stream << e.what() << "\n";
                }
            }
            else if (command == "cd") {
                if (args.size() > 1) {
                    error_stream << "Invalid argument" << "\n";
                    std::exit(1);
                }
                auto argv = args[0];
                const char* home_env = std::getenv("HOME");
                const std::string target_dir = (argv == "~") ? std::string(home_env) : argv;
                if (std::filesystem::exists(target_dir)) {
                    std::filesystem::current_path(target_dir);
                }
                else {
                    error_stream << command << ": " << target_dir << ": No such file or directory" << "\n";
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

                        int fd = open(redirect_filename.c_str(), O_WRONLY | O_TRUNC | O_CREAT, 0777);
                        if (is_redirect == redirect_states::REDIRECT_OUTPUT) {
                            dup2(fd, STDOUT_FILENO);
                        }
                        else if (is_redirect == redirect_states::REDIRECT_ERROR) {
                            dup2(fd, STDERR_FILENO);
                        }
                        close(fd);
                        execv(exe_path.c_str(), parsed_args_ptrs.data());
                    }
                    else if (pid > 0) {
                        int status;
                        waitpid(pid, &status, 0);
                    }
                    else {
                        error_stream << "Fork failed: " << std::strerror(errno) << "\n";
                    }
                }
                else {
                    error_stream << command << ": not found" << "\n";
                }
            }

            if (error_stream.str().length() > 0) handle_redirect(redirect_filename, is_redirect == redirect_states::REDIRECT_ERROR, error_stream);
            else if (out_stream.str().length() > 0) handle_redirect(redirect_filename, is_redirect == redirect_states::REDIRECT_OUTPUT, out_stream);
        }
        catch (std::invalid_argument& e) {
            std::cerr << e.what() << "\n";
            continue;
        }
    }
}
