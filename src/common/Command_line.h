#pragma once
#include <filesystem>
#include <vector>

namespace fs = std::filesystem;

/**
 * Access to the files that were passed to the application, e.g. when a .dcm
 * file is opened from the Windows explorer or dropped on the executable.
 */
namespace Command_line {
    /// Returns the file paths that were passed on the command line.
    /// Arguments starting with '-' are ignored since they are treated as options.
    /// On Windows the wide character command line is used, so that paths with
    /// characters outside of the ANSI code page (e.g. Chinese) work correctly.
    std::vector<fs::path> get_file_paths(int argc, char** argv);
}
