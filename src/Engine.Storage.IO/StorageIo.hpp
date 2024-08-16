#pragma once

#include <Engine.Common/Sal.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Memory/MemoryPointer.hpp>
#include <Engine.Storage/IStorage.hpp>

#include "Io.hpp"
#include "UniqueResourceAccessor.hpp"

namespace hg::engine {
	class Engine;
}

// Warning: Temporary
// Bug: We are required to include the complete types here, as the compiler has problems to resolve the forwarded types in the implementation
#include <Engine.Storage/Storage/LocalFileStorage.hpp>
#include <Engine.Storage/Storage/MemoryStorage.hpp>

namespace hg::engine::resource {
	using MemoryBuffer = storage::system::MemoryStorage::MemoryObject;
	class LocalFile;

	class Archive;
	class Blob;
	class Package;
}

namespace hg::engine::storage {
	class IStorage;
}

namespace hg::engine::storage::system {
	class ArchiveStorage;
	class LocalFileStorage;
	class MemoryStorage;
	class PackageStorage;
}

namespace hg::engine::storage {
	/**/

	using AccessArchiveReadonly = UniqueResourceAccessor<const Io<const resource::Archive>>;
	using AccessArchiveReadWrite = UniqueResourceAccessor<Io<resource::Archive>>;

	using AccessBlobReadonly = UniqueResourceAccessor<const Io<const resource::Blob>>;
	using AccessBlobReadWrite = UniqueResourceAccessor<Io<resource::Blob>>;

	using AccessPackageReadonly = UniqueResourceAccessor<const Io<const resource::Package>>;
	using AccessPackageReadWrite = UniqueResourceAccessor<Io<resource::Package>>;

	/**/

	class StorageIo final {
	public:
		using this_type = StorageIo;

	public:
		explicit StorageIo(_In_ ref<Engine> engine_) noexcept;

		StorageIo(cref<this_type>) = delete;

		StorageIo(mref<this_type>) = delete;

		constexpr ~StorageIo() noexcept = default;

	private:
		nmpt<Engine> _engine;

	public:
		[[nodiscard]] UniqueResourceAccessor<Io<resource::MemoryBuffer>> accessReadWrite(
			_In_ mref<Arci<system::MemoryStorage>> storage_
		) const;

		[[nodiscard]] UniqueResourceAccessor<const Io<const resource::MemoryBuffer>> accessReadonly(
			_In_ mref<Arci<system::MemoryStorage>> storage_
		) const;

		[[nodiscard]] UniqueResourceAccessor<Io<resource::LocalFile>> accessReadWrite(
			_In_ mref<Arci<system::LocalFileStorage>> storage_
		) const;

		[[nodiscard]] UniqueResourceAccessor<const Io<const resource::LocalFile>> accessReadonly(
			_In_ mref<Arci<system::LocalFileStorage>> storage_
		) const;

		[[nodiscard]] AccessArchiveReadWrite accessReadWrite(
			_In_ mref<Arci<system::ArchiveStorage>> storage_
		) const;

		[[nodiscard]] AccessArchiveReadonly accessReadonly(
			_In_ mref<Arci<system::ArchiveStorage>> storage_
		) const;

		[[nodiscard]] AccessPackageReadWrite accessReadWrite(
			_In_ mref<Arci<system::PackageStorage>> storage_
		) const;

		[[nodiscard]] AccessPackageReadonly accessReadonly(
			_In_ mref<Arci<system::PackageStorage>> storage_
		) const;

	public:
		[[nodiscard]] AccessBlobReadonly accessReadonlyBlob(
			_In_ mref<Arci<system::MemoryStorage>> storage_
		) const;

		[[nodiscard]] AccessBlobReadonly accessReadonlyBlob(
			_In_ mref<Arci<system::LocalFileStorage>> storage_
		) const;

		[[nodiscard]] AccessBlobReadonly accessReadonlyBlob(
			_In_ mref<Arci<IStorage>> storage_
		) const;

		[[nodiscard]] AccessBlobReadWrite accessWriteonlyBlob(
			_In_ mref<Arci<system::MemoryStorage>> storage_
		) const;

		[[nodiscard]] AccessBlobReadWrite accessWriteonlyBlob(
			_In_ mref<Arci<system::LocalFileStorage>> storage_
		) const;

		[[nodiscard]] AccessBlobReadWrite accessWriteonlyBlob(
			_In_ mref<Arci<IStorage>> storage_
		) const;

		[[nodiscard]] AccessBlobReadWrite accessReadWriteBlob(
			_In_ mref<Arci<system::MemoryStorage>> storage_
		) const;

		[[nodiscard]] AccessBlobReadWrite accessReadWriteBlob(
			_In_ mref<Arci<system::LocalFileStorage>> storage_
		) const;

		[[nodiscard]] AccessBlobReadWrite accessReadWriteBlob(
			_In_ mref<Arci<IStorage>> storage_
		) const;
	};
}
