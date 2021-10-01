#include "ImporterManager.hpp"

using namespace ember::engine::res;
using namespace ember;

ImporterManager::~ImporterManager() noexcept {
    tidy();
}

void ImporterManager::tidy() {

    for (const auto& entry : _mapping) {
        delete entry.importer;
    }

    _mapping.clear();
}

bool ImporterManager::registerImporter(cref<FileTypeId> fileTypeId_, ptr<ImporterBase> importer_) noexcept {
    return _mapping.emplace({ fileTypeId_, importer_ }).second;
}

bool ImporterManager::unregisterImporter(ptr<ImporterBase> importer_) noexcept {
    throw NotImplementedException();
}

bool ImporterManager::unregisterImporter(cref<FileTypeId> fileTypeId_) noexcept {
    throw NotImplementedException();
}

ptr<ImporterBase> ImporterManager::importer(cref<FileTypeId> fileType_, cref<File> file_) const {

    auto mapped = _mapping.find({ fileType_, nullptr });
    if (mapped == _mapping.cend()) {
        return nullptr;
    }

    if (mapped.value().importer->canImport(fileType_, file_)) {
        return mapped.value().importer;
    }

    return nullptr;
}
