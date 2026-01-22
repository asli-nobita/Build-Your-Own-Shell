#include "helper.h" 
#ifdef _WIN32 
constexpr char PATH_LIST_SEPARATOR = ';';
#else 
constexpr char PATH_LIST_SEPARATOR = ':';
#endif

enum class State {
    START,
    IN_DOUBLE_QUOTES,
    IN_SINGLE_QUOTES,
    IN_TEXT
};


const std::pair<std::string, std::vector<std::string>> parse_command(std::string input) {
    std::istringstream iss(input);
    std::string first;
    iss >> first;
    std::string rest;
    std::getline(iss, rest);
    // start reading input 
    std::vector<std::string> args;
    State cur_state = State::START;
    std::string cur_token;
    for (char c : rest) {
        switch (cur_state) {
            case State::START:
                if (std::isspace(c)) {
                    // if (!cur_token.empty()) {
                    //     args.push_back(cur_token);
                    //     cur_token.clear();
                    // }
                    cur_state = State::START;
                }
                else {
                    if (c == '\'') {
                        cur_state = State::IN_SINGLE_QUOTES;
                    }
                    else if (c == '\"') {
                        cur_state = State::IN_DOUBLE_QUOTES;
                    }
                    else {
                        cur_state = State::IN_TEXT;
                        cur_token += c;
                    }
                }
                break;
            case State::IN_TEXT:
                if (std::isspace(c)) {
                    args.push_back(cur_token);
                    cur_token.clear();
                    cur_state = State::START;
                }
                else {
                    if (c == '\'') {
                        cur_state = State::IN_SINGLE_QUOTES;
                    }
                    else if(c == '\"') { 
                        cur_state = State::IN_DOUBLE_QUOTES;
                    }
                    else {
                        cur_token += c;
                        cur_state = State::IN_TEXT;
                    }
                }
                break;
            case State::IN_SINGLE_QUOTES:
                if (c == '\'') {
                    cur_state = State::IN_TEXT;
                }
                else {
                    cur_token += c;
                    cur_state = State::IN_SINGLE_QUOTES;
                }
                break; 
            case State::IN_DOUBLE_QUOTES:
                if (c == '\"') {
                    cur_state = State::IN_TEXT;
                }
                else {
                    cur_token += c;
                    cur_state = State::IN_DOUBLE_QUOTES;
                }
        }
    }
    if (cur_state == State::IN_SINGLE_QUOTES || cur_state == State::IN_DOUBLE_QUOTES) {
        throw std::invalid_argument("Exception: Missing closing quotes in argument");
    }
    if (!cur_token.empty()) args.push_back(cur_token);

    return { first, args };
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

const std::string search_in_path(const std::string& PATH, std::string& command) {
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

