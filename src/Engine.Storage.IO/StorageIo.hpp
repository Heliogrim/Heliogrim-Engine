#pragma once

#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Memory/MemoryPointer.hpp>
#include <Engine.Storage/IStorage.hpp>

#include "Io.hpp"

namespace hg {
	template <typename ManagedType_>
	class ResourceAccessor;
}

namespace hg::engine {
	class Engine;
}

namespace hg::engine::resource {
	class MemoryBuffer;
	class LocalFile;
	class Package;
}

namespace hg::engine::serialization {
	class Archive;
}

namespace hg::engine::storage {
	class IStorage;
	class StorageInputStream;
	class StorageOutputStream;
}

namespace hg::engine::storage::system {
	class ArchiveStorage;
	class LocalFileStorage;
	class MemoryStorage;
	class PackageStorage;
}

namespace hg::engine::storage {
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
		[[nodiscard]] uptr<ResourceAccessor<Io<resource::MemoryBuffer>>, decltype([](auto*) {})> accessReadWrite(
			_In_ mref<Arci<system::MemoryStorage>> storage_
		);

		[[nodiscard]] uptr<ResourceAccessor<Io<const resource::MemoryBuffer>>, decltype([](auto*) {})> accessReadonly(
			_In_ mref<Arci<system::MemoryStorage>> storage_
		);

		[[nodiscard]] uptr<ResourceAccessor<Io<resource::LocalFile>>, decltype([](auto*) {})> accessReadWrite(
			_In_ mref<Arci<system::LocalFileStorage>> storage_
		);

		[[nodiscard]] uptr<ResourceAccessor<Io<const resource::LocalFile>>, decltype([](auto*) {})> accessReadonly(
			_In_ mref<Arci<system::LocalFileStorage>> storage_
		);

		[[nodiscard]] uptr<ResourceAccessor<Io<serialization::Archive>>, decltype([](auto*) {})> accessReadWrite(
			_In_ mref<Arci<system::ArchiveStorage>> storage_
		);

		[[nodiscard]] uptr<ResourceAccessor<Io<const serialization::Archive>>, decltype([](auto*) {})> accessReadonly(
			_In_ mref<Arci<system::ArchiveStorage>> storage_
		);

		[[nodiscard]] uptr<ResourceAccessor<Io<resource::Package>>, decltype([](auto*) {})> accessReadWrite(
			_In_ mref<Arci<system::PackageStorage>> storage_
		);

		[[nodiscard]] uptr<ResourceAccessor<Io<const resource::Package>>, decltype([](auto*) {})> accessReadonly(
			_In_ mref<Arci<system::PackageStorage>> storage_
		);
	};
}
