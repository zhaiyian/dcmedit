#pragma once
#include "models/Dicom_files.h"
#include "ui/open_files_dialog/IOpen_files_view.h"

#include <filesystem>
#include <vector>

namespace fs = std::filesystem;

class Open_files_presenter
{
public:
    Open_files_presenter(IOpen_files_view&, Dicom_files&);

    void show_dialog();
    /// Opens the given files without asking the user for them.
    void open_files(const std::vector<fs::path>&);

private:
    IOpen_files_view& m_view;
    Dicom_files& m_dicom_files;
};
