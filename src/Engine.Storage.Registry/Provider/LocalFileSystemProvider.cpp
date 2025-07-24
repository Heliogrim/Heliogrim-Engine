#include "LocalFileSystemProvider.hpp"

#include <Engine.Common/Make.hpp>
#include <Engine.Config/Config.hpp>
#include <Engine.Config/Enums.hpp>
#include <Engine.Reflect/Cast.hpp>

#include "../IStorage.hpp"
#include "../IStorageFileRepository.hpp"
#include "../Repository/CacheFileSystemRepository.hpp"
#include "../Repository/EditorReadOnlyRepository.hpp"
#include "../Repository/ProjectFileSystemRepository.hpp"
#include "../Repository/RuntimeFileSystemRepository.hpp"
#include "../Storage/LocalFileStorage.hpp"

using namespace hg::engine::storage::system;
using namespace hg;

/**/

LocalFileSystemProvider::LocalFileSystemProvider() noexcept = default;

LocalFileSystemProvider::~LocalFileSystemProvider() noexcept = default;

template <typename RepType_>
nmpt<RepType_> LocalFileSystemProvider::makeUniqueRepository(
	auto& repositories_,
	mref<hg::fs::File::path_type> basePath_
) {

	constexpr auto repoTypeId = ::hg::refl::TypeId<RepType_>();
	for (auto& entry : repositories_) {
		if (entry.typeId != repoTypeId) {
			continue;
		}

		auto* const rep = static_cast<ptr<RepType_>>(entry.repository.get());
		if (rep->_basePath == basePath_) {
			return { rep };
		}
	}

	return static_cast<ptr<RepType_>>(repositories_.emplace_back(
		repoTypeId,
		uptr<RepType_> { new RepType_(*this, std::move(basePath_)) }
	).repository.get());
}

Arci<LocalFileStorage> LocalFileSystemProvider::makeStorageObject(mref<fs::Path> storagePath_) const noexcept {
	// TODO: Rework
	constexpr bool writeable = true;
	constexpr bool readable = true;
	return Arci<LocalFileStorage>::create(std::move(storagePath_), readable, writeable, readable, writeable);
}

nmpt<CacheFileSystemRepository> LocalFileSystemProvider::makeCacheRepository(
	mref<hg::fs::File::path_type> basePath_
) {
	return makeUniqueRepository<CacheFileSystemRepository>(_repositories, std::move(basePath_));
}

nmpt<ProjectFileSystemRepository> LocalFileSystemProvider::makeProjectRepository(
	mref<hg::fs::File::path_type> basePath_
) {
	return makeUniqueRepository<ProjectFileSystemRepository>(_repositories, std::move(basePath_));
}

nmpt<RuntimeFileSystemRepository> LocalFileSystemProvider::makeRuntimeRepository(
	mref<hg::fs::File::path_type> basePath_
) {
	return makeUniqueRepository<RuntimeFileSystemRepository>(_repositories, std::move(basePath_));
}

nmpt<EditorReadOnlyRepository> LocalFileSystemProvider::makeEditorRepository(
	mref<hg::fs::File::path_type> basePath_
) {
	return makeUniqueRepository<EditorReadOnlyRepository>(_repositories, std::move(basePath_));
}
