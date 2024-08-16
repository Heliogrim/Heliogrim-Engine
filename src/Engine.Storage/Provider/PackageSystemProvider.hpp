#pragma once

#include <Engine.Common/Sal.hpp>
#include <Engine.Common/Collection/InlineAutoArray.hpp>
#include <Engine.Common/Managed/Rc.hpp>
#include <Engine.Common/Memory/MemoryPointer.hpp>

#include "../IStorageProvider.hpp"

namespace hg::engine::storage {
	class IStorageRegistry;
	class ArchiveStorageDescriptor;
	class PackageStorageDescriptor;
}

namespace hg::engine::storage::system {
	class ArchiveStorage;
	class ArchiveSystemRepository;
	class PackageStorage;
	class PackageSystemRepository;
}

namespace hg::engine::storage::system {
	class PackageSystemProvider final :
		public IStorageProvider {
	public:
		friend class ArchiveSystemRepository;
		friend class PackageSystemRepository;

	public:
		using this_type = PackageSystemProvider;

	public:
		explicit PackageSystemProvider(_In_ ref<IStorageRegistry> registry_) noexcept;

		~PackageSystemProvider() noexcept override;

	private:
		nmpt<IStorageRegistry> _registry;
		InlineAutoArray<uptr<PackageSystemRepository>> _packageRepositories;
		InlineAutoArray<uptr<ArchiveSystemRepository>> _archiveRepositories;

	protected:
		[[nodiscard]] Arci<ArchiveStorage> makeStorageObject(mref<ArchiveStorageDescriptor> descriptor_);

		[[nodiscard]] Arci<PackageStorage> makeStorageObject(mref<PackageStorageDescriptor> descriptor_);

	public:
		[[nodiscard]] nmpt<PackageSystemRepository> makePackageRepository();

		[[nodiscard]] nmpt<ArchiveSystemRepository> makeArchiveRepository();
	};
}
