#pragma once

#include <dcmtk/ofstd/offile.h>
#include <filesystem>
#include <string>

namespace fs = std::filesystem;

/**
 * Helpers for handling paths that may contain characters outside of the
 * current locale/code page, e.g. Chinese directory names.
 *
 * Converting a path to a narrow string (or creating a path from a narrow
 * string) is locale dependent on Windows. Characters that cannot be
 * represented in the current code page either throw a std::filesystem::filesystem_error
 * or get silently mangled, which both makes files impossible to open and can
 * crash the application. These helpers use UTF-8 instead, which does not
 * depend on the locale and is also the encoding used by QString::toStdString()
 * and QString::fromStdString().
 */
namespace Path_util {
    /// Convert a path to a UTF-8 encoded string. Never depends on the locale.
    std::string to_utf8(const fs::path& path);

    /// Create a path from a UTF-8 encoded string.
    fs::path from_utf8(const std::string& utf8_string);

    /// Convert a path to a filename that can be passed to DCMTK file functions.
    OFFilename to_offilename(const fs::path& path);
}
