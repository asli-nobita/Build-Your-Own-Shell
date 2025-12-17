#include "helper.h" 

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
