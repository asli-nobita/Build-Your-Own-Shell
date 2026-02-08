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
            auto cmd = parse_command(input);

            std::ostringstream out_stream, error_stream;

            if (cmd.command == "type") {
                // command as argument should be single word
                if (cmd.args.empty()) error_stream << ": not found\n";
                else if (cmd.args.size() > 1) std::cout << "Invalid argument" << "\n";
                else {
                    auto argv = cmd.args[0];
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
            else if (cmd.command == "echo") {
                // concatenate all arguments and print to out_stream 
                for (auto& arg : cmd.args) {
                    out_stream << arg << " ";
                }
                out_stream << "\n";
            }
            else if (cmd.command == "exit") {
                std::exit(0);
            }
            else if (cmd.command == "pwd") {
                try {
                    std::filesystem::path current_dir = std::filesystem::current_path();
                    out_stream << current_dir.string() << "\n";
                }
                catch (const std::filesystem::filesystem_error& e) {
                    error_stream << e.what() << "\n";
                }
            }
            else if (cmd.command == "cd") {
                if (cmd.args.size() > 1) {
                    error_stream << "Invalid argument" << "\n";
                    std::exit(1);
                }
                auto argv = cmd.args[0];
                const char* home_env = std::getenv("HOME");
                const std::string target_dir = (argv == "~") ? std::string(home_env) : argv;
                if (std::filesystem::exists(target_dir)) {
                    std::filesystem::current_path(target_dir);
                }
                else {
                    error_stream << cmd.command << ": " << target_dir << ": No such file or directory" << "\n";
                }
            }
            else {
                // search for executable cmd.command in PATH 
                auto exe_path = search_in_path(PATH, cmd.command);
                if (!exe_path.empty()) {
                    pid_t pid = fork();
                    if (pid == 0) {
                        // child process 
                        // execv expects a argument list of char pointers 
                        // .c_str() returns const pointers, so we need a cast
                        std::vector<char*> parsed_args_ptrs;
                        // first argument should be name of executable 
                        parsed_args_ptrs.push_back(const_cast<char*>(cmd.command.c_str()));
                        for (auto& arg : cmd.args) parsed_args_ptrs.push_back(const_cast<char*>(arg.c_str()));
                        parsed_args_ptrs.push_back(nullptr);

                        if (cmd.rd_mode != redirect_mode::NO_REDIRECT) {
                            auto flags = O_CREAT | O_WRONLY;
                            int fd;
                            if (cmd.rd_mode == redirect_mode::REDIRECT_ERROR) {
                                fd = open(cmd.redirect_filename.c_str(), flags | O_TRUNC, 0777);
                                dup2(fd, STDERR_FILENO);
                            }
                            else if (cmd.rd_mode == redirect_mode::APPEND_ERROR) {
                                fd = open(cmd.redirect_filename.c_str(), flags | O_APPEND, 0777);
                                dup2(fd, STDERR_FILENO);
                            }
                            else if (cmd.rd_mode == redirect_mode::REDIRECT_OUTPUT) {
                                fd = open(cmd.redirect_filename.c_str(), flags | O_TRUNC, 0777);
                                dup2(fd, STDOUT_FILENO);
                            }
                            else if (cmd.rd_mode == redirect_mode::APPEND_OUTPUT) {
                                fd = open(cmd.redirect_filename.c_str(), flags | O_APPEND, 0777);
                                dup2(fd, STDOUT_FILENO);
                            }
                            close(fd); 
                        }

                        execv(exe_path.c_str(), parsed_args_ptrs.data());
                    }
                    else if (pid > 0) {
                        int status;
                        waitpid(pid, &status, 0);
                        // child already wrote to the file, so we can disable redirect for parent 
                        cmd.rd_mode = redirect_mode::NO_REDIRECT; 
                    }
                    else {
                        error_stream << "Fork failed: " << std::strerror(errno) << "\n";
                    }
                }
                else {
                    error_stream << cmd.command << ": not found" << "\n";
                }
            }

            handle_redirect(cmd, out_stream, error_stream);
        }
        catch (std::invalid_argument& e) {
            std::cerr << e.what() << "\n";
            continue;
        }
    }
}
