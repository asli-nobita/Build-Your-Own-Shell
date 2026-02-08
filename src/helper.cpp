#include "helper.h" 

Command parse_command(const std::string& input) {
    std::istringstream iss(input);
    std::string cmd_string;
    std::getline(iss, cmd_string);

    Command cmd;

    // start reading input 
    std::vector<std::string> parsed_cmd;
    command_states cur_state = command_states::START;
    std::string cur_token;
    std::unordered_set<char> can_escape{ '\"', '\\', '$', '`', '\n' };
    auto len = cmd_string.length();
    auto to_escape = false;

    for (unsigned int i = 0; i < len; i++) {
        auto c = cmd_string[i];
        switch (cur_state) {
            case command_states::START:
                if (std::isspace(c)) {
                    cur_state = command_states::START;
                }
                else {
                    if (c == '\'' && !to_escape) {
                        cur_state = command_states::IN_SINGLE_QUOTES;
                    }
                    else if (c == '\"' && !to_escape) {
                        cur_state = command_states::IN_DOUBLE_QUOTES;
                    }
                    else if (c == '\\' && !to_escape) {
                        to_escape = true;
                    }
                    else {
                        if (to_escape) to_escape = false;
                        cur_state = command_states::IN_TEXT;
                        cur_token += c;
                    }
                }
                break;
            case command_states::IN_TEXT:
                if (std::isspace(c) && !to_escape) {
                    if (cur_token == commands::redirect_output || cur_token == commands::redirect_std_output) {
                        cmd.rd_mode = redirect_mode::REDIRECT_OUTPUT;
                        cur_state = command_states::IN_TEXT;
                        cur_token.clear();
                        continue;
                    }
                    else if (cur_token == commands::append_output || cur_token == commands::append_std_output) {
                        cmd.rd_mode = redirect_mode::APPEND_OUTPUT;
                        cur_state = command_states::IN_TEXT;
                        cur_token.clear();
                        continue;
                    }
                    else if (cur_token == commands::append_error) {
                        cmd.rd_mode = redirect_mode::APPEND_ERROR;
                        cur_state = command_states::IN_TEXT;
                        cur_token.clear();
                        continue;
                    }
                    else if (cur_token == commands::redirect_error) {
                        cmd.rd_mode = redirect_mode::REDIRECT_ERROR;
                        cur_state = command_states::IN_TEXT;
                        cur_token.clear();
                        continue;
                    }
                    parsed_cmd.push_back(cur_token);
                    cur_token.clear();
                    cur_state = command_states::START;
                }
                else {
                    if (c == '\'' && !to_escape) {
                        cur_state = command_states::IN_SINGLE_QUOTES;
                    }
                    else if (c == '\"' && !to_escape) {
                        cur_state = command_states::IN_DOUBLE_QUOTES;
                    }
                    else if (c == '\\' && !to_escape) {
                        to_escape = true;
                    }
                    else {
                        if (to_escape) to_escape = false;
                        cur_token += c;
                        cur_state = command_states::IN_TEXT;
                    }
                }
                break;
            case command_states::IN_SINGLE_QUOTES:
                if (c == '\'' && !to_escape) {
                    cur_state = command_states::IN_TEXT;
                }
                else {
                    if (to_escape) to_escape = false;
                    cur_token += c;
                    cur_state = command_states::IN_SINGLE_QUOTES;
                }
                break;
            case command_states::IN_DOUBLE_QUOTES:
                if (c == '\"' && !to_escape) {
                    cur_state = command_states::IN_TEXT;
                }
                else if (c == '\\' && i < len - 1 && can_escape.count(cmd_string[i + 1]) && !to_escape) {
                    // can only escape specific characters  
                    to_escape = true;
                }
                else {
                    if (to_escape) to_escape = false;
                    cur_token += c;
                    cur_state = command_states::IN_DOUBLE_QUOTES;
                }
                break;
        }
    }
    if (cur_state == command_states::IN_SINGLE_QUOTES || cur_state == command_states::IN_DOUBLE_QUOTES) {
        throw std::invalid_argument("Exception: Missing closing quotes in argument");
    }
    if (!cur_token.empty()) parsed_cmd.push_back(cur_token);

    if (!parsed_cmd.empty()) {
        cmd.command = parsed_cmd[0];
        cmd.args = std::vector(parsed_cmd.begin() + 1, parsed_cmd.end());
        if (cmd.rd_mode != redirect_mode::NO_REDIRECT) {
            cmd.redirect_filename = cmd.args.back();
            cmd.args.pop_back();
        }
    }

    return cmd;
}

void handle_redirect(const Command& cmd, std::ostringstream& output_stream, std::ostringstream& error_stream) {
    // open or create file in write mode and write output to file
    if (cmd.rd_mode == redirect_mode::NO_REDIRECT) {
            if(!error_stream.str().empty()) std::cerr << error_stream.str();
            if(!output_stream.str().empty()) std::cout << output_stream.str();
        return;
    }
    auto flags = O_CREAT | O_WRONLY;
    int fd;
    if (cmd.rd_mode == redirect_mode::REDIRECT_ERROR) {
        fd = open(cmd.redirect_filename.c_str(), flags | O_TRUNC, 0777);
    }
    else if (cmd.rd_mode == redirect_mode::APPEND_ERROR) {
        fd = open(cmd.redirect_filename.c_str(), flags | O_APPEND, 0777);
    }
    else if (cmd.rd_mode == redirect_mode::REDIRECT_OUTPUT) {
        fd = open(cmd.redirect_filename.c_str(), flags | O_TRUNC, 0777);
    }
    else if (cmd.rd_mode == redirect_mode::APPEND_OUTPUT) {
        fd = open(cmd.redirect_filename.c_str(), flags | O_APPEND, 0777);
    }

    std::string buf;
    if(cmd.rd_mode == redirect_mode::APPEND_ERROR || cmd.rd_mode == redirect_mode::REDIRECT_ERROR) { 
        buf = error_stream.str(); 
        if(!output_stream.str().empty()) std::cout << output_stream.str(); 
    } 
    else { 
        buf = output_stream.str(); 
        if(!error_stream.str().empty()) std::cerr << error_stream.str();
    }
    auto NBYTES = buf.length();
    if (NBYTES > 0) {
        auto bytes = write(fd, buf.c_str(), NBYTES);
        if (bytes == -1) {
            std::cerr << "Error writing to file\n";
            return;
        }
        // std::cout << "Debugging: " << bytes << " bytes written to file" << std::endl;
    }
    close(fd);
}


// string processing functions
void ltrim(std::string& s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
        return !std::isspace(ch);
        }));
}

void rtrim(std::string& s) {
    s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
        return !std::isspace(ch);
        }).base(), s.end());
}

void trim(std::string& s) {
    ltrim(s);
    rtrim(s);
}

bool find_executable(const std::string& full_path) {
    // we need to check whether 1) this path exists and 2) a executable file is present at this path 
    if (!std::filesystem::exists(full_path) || !std::filesystem::is_regular_file(full_path)) return false;
    // get file permissions as a bitmask
    auto perms = std::filesystem::status(full_path).permissions();
    return (perms & (std::filesystem::perms::owner_exec |
        std::filesystem::perms::group_exec |
        std::filesystem::perms::others_exec)) != std::filesystem::perms::none;
}

const std::string search_in_path(const std::string& PATH, const std::string& command) {
    std::vector<std::string> path_directories;
    std::istringstream iss(PATH);
    std::string dir;
    std::string full_path;
    while (std::getline(iss, dir, PATH_LIST_SEPARATOR)) {
        path_directories.push_back(dir);
    }

    for (auto& dir_name : path_directories) {
        // this concatenates the dir and the command name to get a full path 
        full_path = (std::filesystem::path(dir_name) / command).string();
        if (find_executable(full_path)) {
            return full_path;
        }
    }

    return "";
}

