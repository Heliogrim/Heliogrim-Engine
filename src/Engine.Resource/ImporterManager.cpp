#include "ImporterManager.hpp"

using namespace ember::engine::res;
using namespace ember;

ImporterManager::~ImporterManager() noexcept {
    tidy();
}

void ImporterManager::tidy() {

    for (const auto& entry : _mapping) {
        delete entry.second;
    }

    _mapping.clear();
}

bool ImporterManager::registerImporter(cref<FileTypeId> fileTypeId_, ptr<ImporterBase> importer_) noexcept {
    return _mapping.insert({ fileTypeId_, importer_ }).second;
}

bool ImporterManager::unregisterImporter(ptr<ImporterBase> importer_) noexcept {
    throw NotImplementedException();
}

bool ImporterManager::unregisterImporter(cref<FileTypeId> fileTypeId_) noexcept {
    throw NotImplementedException();
}

ptr<ImporterBase> ImporterManager::importer(cref<FileTypeId> fileType_, cref<File> file_) const {

    auto mapped = _mapping.find(fileType_);
    if (mapped == _mapping.cend()) {
        return nullptr;
    }

    if (mapped->second->canImport(fileType_, file_)) {
        return mapped->second;
    }

    return nullptr;
}
