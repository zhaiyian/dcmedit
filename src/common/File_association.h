#pragma once

/**
 * Registration of the application as a program that can open .dcm files.
 */
namespace File_association {
    /// Makes the current executable available as an application that can open
    /// .dcm files for the current user (HKEY_CURRENT_USER, so no administrator
    /// rights are needed). If no program is associated with the extension yet,
    /// the application becomes the default, which makes double clicking a file
    /// in the explorer work. An association that the user made earlier is never
    /// overwritten. Does nothing on platforms other than Windows.
    void register_for_current_user();
}
