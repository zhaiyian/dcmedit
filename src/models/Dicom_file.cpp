#include "Dicom_file.h"

#include "common/Path_util.h"
#include "logging/Log.h"

#include <dcmtk/dcmdata/dcdeftag.h>
#include <dcmtk/dcmdata/dcmetinf.h>
#include <dcmtk/dcmdata/dcuid.h>
#include <dcmtk/dcmdata/dcxfer.h>
#include <stdexcept>

Dicom_file::Dicom_file(const fs::path& path)
    : m_path(path),
      m_unsaved_changes(false) {
    const OFFilename file_name = Path_util::to_offilename(path);

    if(!OFStandard::fileExists(file_name)) {
        throw std::runtime_error("file not found");
    }
    OFCondition status = m_file.loadFile(file_name);

    if(status.bad()) {
        throw std::runtime_error(status.text());
    }
    Log::debug("Loaded file: " + Path_util::to_utf8(m_path));
}

bool Dicom_file::is_dicomdir() {
    DcmMetaInfo* meta_info = m_file.getMetaInfo();

    if(meta_info == nullptr) {
        return false;
    }

    OFString media_storage;
    meta_info->findAndGetOFString(DCM_MediaStorageSOPClassUID, media_storage);

    return media_storage == UID_MediaStorageDirectoryStorage;
}

void Dicom_file::save_file() {
    save_file_as(m_path);
}

void Dicom_file::save_file_as(const fs::path& path) {
    const OFFilename file_name = Path_util::to_offilename(path);
    OFCondition status = m_file.getDataset()->loadAllDataIntoMemory();

    if(status.good()) {
        E_TransferSyntax original_transfer = m_file.getDataset()->getOriginalXfer();
        E_TransferSyntax new_transfer = original_transfer != EXS_Unknown ? original_transfer : EXS_LittleEndianExplicit;
        status = m_file.saveFile(file_name, new_transfer);
    }
    if(status.bad()) {
        throw std::runtime_error(status.text());
    }
    m_path = path;
    m_unsaved_changes = false;
    Log::debug("Saved file: " + Path_util::to_utf8(path));
}

void Dicom_file::create_new_file(const fs::path& path) {
    DcmFileFormat file;
    OFCondition status = file.saveFile(Path_util::to_offilename(path), EXS_LittleEndianExplicit);

    if(status.bad()) {
        throw std::runtime_error(status.text());
    }
}
