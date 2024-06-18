#include "LocalFileSystemProvider.hpp"

#include <Engine.Common/Make.hpp>
#include <Engine.Config/Config.hpp>
#include <Engine.Config/Enums.hpp>

#include "../IStorage.hpp"
#include "../IStorageFileRepository.hpp"
#include "../Repository/CacheFileSystemRepository.hpp"
#include "../Repository/EditorReadOnlyRepository.hpp"
#include "../Repository/ProjectFileSystemRepository.hpp"
#include "../Repository/RuntimeFileSystemRepository.hpp"
#include "../Storage/LocalFileStorage.hpp"

using namespace hg::engine::storage::system;
using namespace hg;

LocalFileSystemProvider::LocalFileSystemProvider() noexcept = default;

LocalFileSystemProvider::~LocalFileSystemProvider() noexcept = default;

Arci<LocalFileStorage> LocalFileSystemProvider::makeStorageObject() const noexcept {
	return Arci<LocalFileStorage>::create(false, false, false, false);
}

nmpt<CacheFileSystemRepository> LocalFileSystemProvider::makeCacheRepository(
	mref<hg::fs::File::path_type> basePath_
) {
	return static_cast<ptr<CacheFileSystemRepository>>(_repositories.emplace_back(
		new CacheFileSystemRepository(*this, std::move(basePath_))
	).get());
}

nmpt<ProjectFileSystemRepository> LocalFileSystemProvider::makeProjectRepository(
	mref<hg::fs::File::path_type> basePath_
) {
	return static_cast<ptr<ProjectFileSystemRepository>>(_repositories.emplace_back(
		new ProjectFileSystemRepository(*this, std::move(basePath_))
	).get());
}

nmpt<RuntimeFileSystemRepository> LocalFileSystemProvider::makeRuntimeRepository(
	mref<hg::fs::File::path_type> basePath_
) {
	return static_cast<ptr<RuntimeFileSystemRepository>>(_repositories.emplace_back(
		new RuntimeFileSystemRepository(*this, std::move(basePath_))
	).get());
}

nmpt<EditorReadOnlyRepository> LocalFileSystemProvider::makeEditorRepository(
	mref<hg::fs::File::path_type> basePath_
) {
	return static_cast<ptr<EditorReadOnlyRepository>>(_repositories.emplace_back(
		new EditorReadOnlyRepository(*this, std::move(basePath_))
	).get());
}
