#ifndef HELPER_H 
#define HELPER_H

#include <iostream>
#include <sstream> 
#include <string>
#include <cstdlib>
#include <algorithm>
#include <vector>
#include <unordered_set>
#include <filesystem>
#include <iomanip>
#include <sys/wait.h> 
#include <unistd.h>
#include <cstring> 
#include <cerrno>
#include <stdexcept>
#include <span>
#include <fstream> 
#include <fcntl.h>
#include <unistd.h>

#ifdef _WIN32 
constexpr char PATH_LIST_SEPARATOR = ';';
#else 
constexpr char PATH_LIST_SEPARATOR = ':';
#endif

namespace commands {
    inline constexpr std::string_view redirect_output = ">";
    inline constexpr std::string_view redirect_std_output = "1>";
    inline constexpr std::string_view redirect_error = "2>";
    inline constexpr std::string_view append_output = ">>";
    inline constexpr std::string_view append_std_output = "1>>";
    inline constexpr std::string_view append_error = "2>>"; 
};

enum class command_states {
    START,
    IN_DOUBLE_QUOTES,
    IN_SINGLE_QUOTES,
    IN_TEXT,
};

enum class input_states {
    IN_COMMAND,
    IN_FILENAME
};

enum class redirect_mode {
    NO_REDIRECT,
    REDIRECT_OUTPUT,
    APPEND_OUTPUT,
    REDIRECT_ERROR, 
    APPEND_ERROR
};

struct Command {
    std::string command;
    std::vector<std::string> args;
    std::string redirect_filename;
    redirect_mode rd_mode = redirect_mode::NO_REDIRECT;
};

Command parse_command(const std::string& input);
void trim(std::string& s);
const std::string search_in_path(const std::string& PATH, const std::string& command);
void setup_fd(const Command& cmd);
void handle_redirect(const Command& cmd, std::ostringstream& output_stream, std::ostringstream& error_stream);

#endif