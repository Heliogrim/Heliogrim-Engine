#include "PackageSystemProvider.hpp"

#include "../Repository/PackageSystemRepository.hpp"
#include "Engine.Storage/Repository/ArchiveSystemRepository.hpp"

using namespace hg::engine::storage::system;
using namespace hg;

PackageSystemProvider::PackageSystemProvider(ref<IStorageRegistry> registry_) noexcept :
	_registry(std::addressof(registry_)) {}

PackageSystemProvider::~PackageSystemProvider() noexcept = default;

nmpt<PackageSystemRepository> PackageSystemProvider::makePackageRepository() {
	::hg::assertrt(_packageRepositories.empty());
	_packageRepositories.emplace_back(new PackageSystemRepository(*this));
	return _packageRepositories.front().get();
}

nmpt<ArchiveSystemRepository> PackageSystemProvider::makeArchiveRepository() {
	::hg::assertrt(_archiveRepositories.empty());
	_archiveRepositories.emplace_back(new ArchiveSystemRepository(*this));
	return _archiveRepositories.front().get();
}
