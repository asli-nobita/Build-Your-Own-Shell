#ifndef HELPER_H 
#define HELPER_H

#include <iostream>
#include <sstream> 
#include <string>
#include <cstdlib>
#include <algorithm>
#include <vector>
#include <filesystem>
#include <iomanip>
#include <sys/wait.h> 
#include <unistd.h>
#include <cstring> 
#include <cerrno>

std::string get_command(std::string input); 
std::string get_command_arguments(std::string input); 
void trim(std::string& s);  
const std::string& search_in_path(const std::string& PATH, std::string& command); 

#endif