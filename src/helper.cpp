#include "helper.h" 
#ifdef _WIN32 
constexpr char PATH_LIST_SEPARATOR = ';';
#else 
constexpr char PATH_LIST_SEPARATOR = ':';
#endif


std::string get_command(std::string input) {
    std::istringstream iss(input);
    std::string first;
    iss >> first;
    return first;
}

std::string get_command_arguments(std::string input) {
    std::istringstream iss(input);
    std::string first;
    iss >> first;
    std::string rest;
    std::getline(iss, rest);
    auto pos = rest.find_first_not_of(' ');
    if (pos != std::string::npos) {
        rest.erase(0, pos);
    }
    else {
        rest.clear();
    }
    return rest;
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

