#pragma once

#include <Engine.Common/Sal.hpp>
#include <Engine.Common/Collection/AutoArray.hpp>

#include "../MutableStorageArchive.hpp"

namespace hg::engine::resource {
	class ReadWritePackage;
}

namespace hg::engine::resource::package {
	struct ReadWriteLinkedArchive;
}

namespace hg::engine::resource {
	class PackageReadWriteArchive final :
		public InheritMeta<PackageReadWriteArchive, MutableStorageArchive> {
	public:
		using this_type = PackageReadWriteArchive;

	public:
		PackageReadWriteArchive(
			_Inout_ ref<ReadWritePackage> package_,
			_Inout_ ref<package::ReadWriteLinkedArchive> linked_
		);

		~PackageReadWriteArchive() noexcept override = default;

	private:
		ref<ReadWritePackage> _package;
		ref<package::ReadWriteLinkedArchive> _linked;
		AutoArray<_::byte> _buffer;

	public:
		[[nodiscard]] cref<ReadWritePackage> getPackage() const noexcept;

		[[nodiscard]] ref<package::ReadWriteLinkedArchive> getLinked() noexcept;

	public:
		void seek(s64 pos_) override;

		[[nodiscard]] s64 tell() const noexcept override;

		[[nodiscard]] s64 totalSize() const noexcept override;

	public:
		void serializeBytes(const ptr<void> value_, u64 size_, const ArchiveStreamMode mode_) override;

	public:
		void preload(streamsize size_);
	};
}
