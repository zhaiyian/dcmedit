#include "ui/open_files_dialog/Open_files_presenter.h"

#include "common/Path_util.h"
#include "models/Dicom_files.h"
#include "ui/open_files_dialog/IOpen_files_view.h"
#include "ui/progressbar/Progress_presenter.h"

#include <eventi/Scoped_defer.h>
#include <filesystem>
#include <memory>
#include <string>
#include <vector>

namespace fs = std::filesystem;

Open_files_presenter::Open_files_presenter(IOpen_files_view& view, Dicom_files& dicom_files)
    : m_view(view),
      m_dicom_files(dicom_files) {}

void Open_files_presenter::show_dialog() {
    open_files(m_view.show_file_dialog());
}

void Open_files_presenter::open_files(const std::vector<fs::path>& file_paths) {
    if (file_paths.empty()) {
        return;
    }
    std::vector<std::string> file_errors;
    eventi::Scoped_defer defer(m_dicom_files.current_file_set);
    std::unique_ptr<IProgress_view> progress_view = m_view.create_progress_view();
    Progress_presenter progress_presenter(*progress_view, "Opening files");
    auto thread_func = [&] {
        progress_presenter.set_max_progress(static_cast<int>(file_paths.size()));
        for(const fs::path& file_path : file_paths) {
            if(progress_presenter.cancelled()) {
                break;
            }
            try {
                m_dicom_files.open_file(file_path);
            }
            catch(const std::exception& e) {
                file_errors.push_back("Failed to open file: " + Path_util::to_utf8(file_path) +
                    "\nReason: " + std::string(e.what()));
            }
            progress_presenter.increment_progress();
        }
        progress_presenter.close();
    };
    progress_presenter.execute(thread_func);

    if(!file_errors.empty()) {
        m_view.show_error(file_errors);
    }
}
