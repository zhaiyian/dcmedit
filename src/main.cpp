#include "common/App_info.h"
#include "common/Command_line.h"
#include "common/File_association.h"
#include "logging/Console_logger.h"
#include "logging/Log.h"
#include "ui/main_view/Main_presenter.h"
#include "ui/main_view/Main_view.h"

#include <QApplication>
#include <memory>
#include <string>
#include <vector>

int main(int argc, char** argv) {
    Log log(Log_level::info);
    log.add_logger(std::make_unique<Console_logger>());

    Log::info("dcmedit " + std::string(App_info::version));

    File_association::register_for_current_user();
    const std::vector<fs::path> file_paths = Command_line::get_file_paths(argc, argv);

    QApplication app(argc, argv);
    Main_view main_view;
    Main_presenter main_presenter(main_view);
    main_view.show();

    if(!file_paths.empty()) {
        main_presenter.open_files(file_paths);
    }
    return app.exec();
}
