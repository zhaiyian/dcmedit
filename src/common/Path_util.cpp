#include "common/Path_util.h"

std::string Path_util::to_utf8(const fs::path& path) {
#if defined(__cpp_lib_char8_t)
    const std::u8string utf8_path = path.u8string();
    return std::string(utf8_path.begin(), utf8_path.end());
#else
    return path.u8string();
#endif
}

fs::path Path_util::from_utf8(const std::string& utf8_string) {
#if defined(__cpp_lib_char8_t)
    return fs::path(std::u8string(utf8_string.begin(), utf8_string.end()));
#else
    return fs::u8path(utf8_string);
#endif
}

OFFilename Path_util::to_offilename(const fs::path& path) {
#if defined(_WIN32) && (defined(WIDE_CHAR_FILE_IO_FUNCTIONS) || defined(WIDE_CHAR_MAIN_FUNCTION))
    // Pass the wide character representation so that DCMTK opens the file with
    // _wfopen() instead of fopen(), which is limited to the ANSI code page.
    return OFFilename(path.wstring().c_str());
#else
    // Other platforms use UTF-8 natively, so the 8-bit representation is fine.
    return OFFilename(to_utf8(path).c_str());
#endif
}
