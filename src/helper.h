#ifndef HELPER_H 
#define HELPER_H

#include <iostream>
#include <sstream> 
#include <string>
#include <algorithm>

std::string get_command(std::string input); 
std::string get_command_arguments(std::string input); 
void trim(std::string& s); 

#endif