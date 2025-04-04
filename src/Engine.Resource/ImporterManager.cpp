#include "ImporterManager.hpp"

using namespace hg::engine::res;
using namespace hg;

ImporterManager::~ImporterManager() noexcept = default;

bool ImporterManager::hasImporter(cref<FileTypeId> fileTypeId_) const noexcept {
	const auto iter = _mapping.find(fileTypeId_);
	return iter != _mapping.end() && iter->second.get() != nullptr;
}

bool ImporterManager::registerImporter(cref<FileTypeId> fileTypeId_, cref<sptr<ImporterBase>> importer_) noexcept {
	return _mapping.insert({ fileTypeId_, importer_ }).second;
}

bool ImporterManager::unregisterImporter(sptr<ImporterBase> importer_) noexcept {

	const auto iter = std::ranges::find_if(
		_mapping,
		[importer_](const auto& pair_) {
			return pair_.second == importer_;
		}
	);

	if (iter == _mapping.cend()) {
		return false;
	}

	// Warning: Potential Memory Leak
	_mapping.erase(iter);
	return true;
}

bool ImporterManager::unregisterImporter(cref<FileTypeId> fileTypeId_) noexcept {

	const auto iter = _mapping.find(fileTypeId_);
	if (iter == _mapping.cend()) {
		return false;
	}

	// Warning: Memory Leak
	_mapping.erase(iter);
	return true;
}

sptr<ImporterBase> ImporterManager::importer(cref<FileTypeId> fileType_, cref<::hg::fs::File> file_) const {

	auto mapped = _mapping.find(fileType_);
	if (mapped == _mapping.cend()) {
		return nullptr;
	}

	if (mapped->second->canImport(fileType_, file_)) {
		return mapped->second;
	}

	return nullptr;
}
