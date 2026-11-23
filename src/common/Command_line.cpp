#include "common/Command_line.h"

#ifdef _WIN32

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <shellapi.h>

#include <string>

std::vector<fs::path> Command_line::get_file_paths(int, char**) {
    std::vector<fs::path> paths;
    int argument_count = 0;
    LPWSTR* arguments = CommandLineToArgvW(GetCommandLineW(), &argument_count);

    if(arguments == nullptr) {
        return paths;
    }
    for(int i = 1; i < argument_count; ++i) {
        const std::wstring argument = arguments[i];

        if(!argument.empty() && argument[0] != L'-') {
            paths.emplace_back(argument);
        }
    }
    LocalFree(arguments);
    return paths;
}

#else

#include <string>

std::vector<fs::path> Command_line::get_file_paths(int argc, char** argv) {
    std::vector<fs::path> paths;

    for(int i = 1; i < argc; ++i) {
        const std::string argument = argv[i];

        if(!argument.empty() && argument[0] != '-') {
            paths.emplace_back(argument);
        }
    }
    return paths;
}

#endif
