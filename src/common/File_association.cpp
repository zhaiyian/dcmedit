#include "common/File_association.h"

#include "logging/Log.h"

#ifdef _WIN32

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <shlobj.h>

#include <cwchar>
#include <string>
#include <vector>

namespace {
const wchar_t* const program_id = L"dcmedit.dcm";
const wchar_t* const extension_name = L".dcm";
const wchar_t* const file_description = L"DICOM image";
const wchar_t* const application_name = L"dcmedit";

std::wstring get_executable_path() {
    std::vector<wchar_t> buffer(MAX_PATH);

    for(;;) {
        const DWORD length = GetModuleFileNameW(nullptr, buffer.data(), static_cast<DWORD>(buffer.size()));

        if(length == 0) {
            return {};
        }
        if(static_cast<size_t>(length) < buffer.size() - 1) {
            return std::wstring(buffer.data(), length);
        }
        buffer.resize(buffer.size() * 2);
    }
}

std::wstring get_executable_name(const std::wstring& path) {
    const auto separator = path.find_last_of(L"\\/");
    return separator == std::wstring::npos ? path : path.substr(separator + 1);
}

/// Reads a string value. Returns false if it is missing or not a string.
bool query_string(HKEY root, const std::wstring& key_path, const wchar_t* value_name, std::wstring& result) {
    HKEY key = nullptr;

    if(RegOpenKeyExW(root, key_path.c_str(), 0, KEY_QUERY_VALUE, &key) != ERROR_SUCCESS) {
        return false;
    }
    DWORD type = 0;
    DWORD size = 0;
    bool found = false;

    if(RegQueryValueExW(key, value_name, nullptr, &type, nullptr, &size) == ERROR_SUCCESS &&
        (type == REG_SZ || type == REG_EXPAND_SZ) && size >= sizeof(wchar_t)) {
        std::vector<wchar_t> buffer(size / sizeof(wchar_t) + 1, L'\0');

        if(RegQueryValueExW(key, value_name, nullptr, &type, reinterpret_cast<BYTE*>(buffer.data()), &size) == ERROR_SUCCESS) {
            result = buffer.data();
            found = true;
        }
    }
    RegCloseKey(key);
    return found;
}

bool value_exists(HKEY root, const std::wstring& key_path, const wchar_t* value_name) {
    HKEY key = nullptr;

    if(RegOpenKeyExW(root, key_path.c_str(), 0, KEY_QUERY_VALUE, &key) != ERROR_SUCCESS) {
        return false;
    }
    const bool exists = RegQueryValueExW(key, value_name, nullptr, nullptr, nullptr, nullptr) == ERROR_SUCCESS;
    RegCloseKey(key);
    return exists;
}

bool write_sz_or_none(HKEY root, const std::wstring& key_path, const wchar_t* value_name, const wchar_t* value) {
    HKEY key = nullptr;

    if(RegCreateKeyExW(root, key_path.c_str(), 0, nullptr, 0, KEY_SET_VALUE, nullptr, &key, nullptr) != ERROR_SUCCESS) {
        return false;
    }
    LONG status = ERROR_SUCCESS;

    if(value == nullptr) {
        status = RegSetValueExW(key, value_name, 0, REG_NONE, nullptr, 0);
    }
    else {
        const DWORD size = static_cast<DWORD>((std::wcslen(value) + 1) * sizeof(wchar_t));
        status = RegSetValueExW(key, value_name, 0, REG_SZ, reinterpret_cast<const BYTE*>(value), size);
    }
    RegCloseKey(key);
    return status == ERROR_SUCCESS;
}

/// Writes the value only if it differs from the current one.
/// Returns true if the registry was modified.
bool write_if_different(HKEY root, const std::wstring& key_path, const wchar_t* value_name, const wchar_t* value) {
    std::wstring current;

    if(query_string(root, key_path, value_name, current) && current == value) {
        return false;
    }
    return write_sz_or_none(root, key_path, value_name, value);
}

/// Adds the program id to a list of programs that can open the extension.
/// Returns true if the registry was modified.
bool add_to_open_with(HKEY root, const std::wstring& key_path) {
    if(value_exists(root, key_path, program_id)) {
        return false;
    }
    return write_sz_or_none(root, key_path, program_id, nullptr);
}
}

void File_association::register_for_current_user() {
    const std::wstring executable_path = get_executable_path();

    if(executable_path.empty()) {
        Log::error("Could not determine the application path for the .dcm file association");
        return;
    }
    const std::wstring classes = L"Software\\Classes\\";
    const std::wstring program_key = classes + program_id;
    const std::wstring extension_key = classes + extension_name;
    const std::wstring application_key = classes + L"Applications\\" + get_executable_name(executable_path);
    const std::wstring command = L"\"" + executable_path + L"\" \"%1\"";
    const std::wstring icon = L"\"" + executable_path + L"\",0";
    bool changed = false;

    // The program id: description, icon and the command used to open a file.
    changed |= write_if_different(HKEY_CURRENT_USER, program_key, nullptr, file_description);
    changed |= write_if_different(HKEY_CURRENT_USER, program_key, L"FriendlyTypeName", file_description);
    changed |= write_if_different(HKEY_CURRENT_USER, program_key + L"\\DefaultIcon", nullptr, icon.c_str());
    changed |= write_if_different(HKEY_CURRENT_USER, program_key + L"\\shell\\open\\command", nullptr, command.c_str());

    // Add the program to the "Open with" list of the extension. Both locations
    // are used by the explorer, depending on the Windows version.
    changed |= add_to_open_with(HKEY_CURRENT_USER, extension_key + L"\\OpenWithProgids");
    changed |= add_to_open_with(HKEY_CURRENT_USER,
        L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\FileExts\\" + std::wstring(extension_name) +
        L"\\OpenWithProgids");

    // An entry in the applications list, so that the program is shown with a
    // proper name in "Open with" and "Choose another app".
    changed |= write_if_different(HKEY_CURRENT_USER, application_key, L"FriendlyAppName", application_name);
    changed |= write_if_different(HKEY_CURRENT_USER, application_key + L"\\shell\\open\\command", nullptr, command.c_str());
    changed |= write_if_different(HKEY_CURRENT_USER, application_key + L"\\SupportedTypes", extension_name, L"");

    // A top level entry in the context menu of the extension, so that a file can
    // be opened with the application without changing the default program.
    const std::wstring context_menu_key = classes + L"SystemFileAssociations\\" + extension_name +
        L"\\shell\\Open with dcmedit";
    changed |= write_if_different(HKEY_CURRENT_USER, context_menu_key, nullptr, L"Open with dcmedit");
    changed |= write_if_different(HKEY_CURRENT_USER, context_menu_key, L"Icon", icon.c_str());
    changed |= write_if_different(HKEY_CURRENT_USER, context_menu_key + L"\\command", nullptr, command.c_str());

    // Only become the default program for the extension if no program is
    // associated with it yet, so that a choice made by the user is kept.
    // Note that the extension is looked up directly below HKEY_CLASSES_ROOT.
    std::wstring current_default;
    query_string(HKEY_CLASSES_ROOT, std::wstring(extension_name), nullptr, current_default);
    if(current_default.empty()) {
        changed |= write_if_different(HKEY_CURRENT_USER, extension_key, nullptr, program_id);
    }
    if(changed) {
        SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, nullptr, nullptr);
        Log::debug("Registered the .dcm file association for the current user");
    }
}

#else

void File_association::register_for_current_user() {}

#endif
