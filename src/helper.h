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

std::string get_command(std::string input); 
std::string get_command_arguments(std::string input); 
void trim(std::string& s);  
void search_in_path(const std::string& PATH, std::string& command); 

#endif