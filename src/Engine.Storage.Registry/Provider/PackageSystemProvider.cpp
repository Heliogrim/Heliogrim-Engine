#include "PackageSystemProvider.hpp"

#include "../Options/ArchiveStorageDescriptor.hpp"
#include "../Options/PackageStorageDescriptor.hpp"
#include "../Repository/ArchiveSystemRepository.hpp"
#include "../Repository/PackageSystemRepository.hpp"
#include "../Storage/ArchiveStorage.hpp"
#include "../Storage/PackageStorage.hpp"

using namespace hg::engine::storage::system;
using namespace hg;

PackageSystemProvider::PackageSystemProvider(ref<IStorageRegistry> registry_) noexcept :
	_registry(std::addressof(registry_)) {}

PackageSystemProvider::~PackageSystemProvider() noexcept = default;

Arci<ArchiveStorage> PackageSystemProvider::makeStorageObject(mref<ArchiveStorageDescriptor> descriptor_) {
	return Arci<ArchiveStorage>::create(
		descriptor_.archiveBacking(),
		descriptor_.archiveGuid(),
		true,
		true,
		true,
		true
	);
}

Arci<PackageStorage> PackageSystemProvider::makeStorageObject(mref<PackageStorageDescriptor> descriptor_) {
	return Arci<PackageStorage>::create(
		std::move(descriptor_).packageBacking(),
		true,
		true,
		true,
		true
	);
}

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
