#include "helper.h" 
#ifdef _WIN32 
constexpr char PATH_LIST_SEPARATOR=';'; 
#else 
constexpr char PATH_LIST_SEPARATOR=':'; 
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

bool find_executable(const std::string& dir_name, const std::string& command) {  
    // this concatenates the dir and the command name to get a full path 
    std::filesystem::path full_path = std::filesystem::path(dir_name) / command; 
    // we need to check if 1) this path exists and 2) a executable file is present at this path 
    return std::filesystem::exists(full_path) && std::filesystem::is_regular_file(full_path); 
}   

void search_in_path(const std::string& PATH, std::string& command) { 
    std::vector<std::string> path_directories; 
    std::istringstream iss(PATH);  
    std::string dir; 
    while(std::getline(iss, dir, PATH_LIST_SEPARATOR)) { 
        path_directories.push_back(dir); 
    } 

    for(auto& dir_name : path_directories) {  
        if(find_executable(dir_name, command)) { 
            std::string full_path = dir_name + "/" + command; 
            std::cout << command << " is " << full_path << std::endl; 
            return; 
        }
    } 
    
    std::cout << command << ": not found" << std::endl;
}
