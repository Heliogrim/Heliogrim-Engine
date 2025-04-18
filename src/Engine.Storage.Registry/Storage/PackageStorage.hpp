#pragma once

#include <Engine.Common/Types.hpp>

#include "../IStorage.hpp"

namespace hg::engine::storage {
	class StorageIo;
}

namespace hg::engine::storage::system {
	class PackageStorage final :
		public InheritMeta<PackageStorage, ISecondaryStorage> {
	public:
		friend class ::hg::engine::storage::StorageIo;

	public:
		using this_type = PackageStorage;

	public:
		PackageStorage() = delete;

		PackageStorage(
			mref<Arci<IStorage>> backing_,
			bool readable_,
			bool writeable_,
			bool randomReadable_,
			bool randomWritable_
		) noexcept;

		~PackageStorage() override = default;

	public:
		[[nodiscard]] constexpr bool isPrimaryStorage() const noexcept override {
			return false;
		}

	private:
		u8 _readable : 1;
		u8 _writeable : 1;
		u8 _randomReadable : 1;
		u8 _randomWritable : 1;

	public:
		[[nodiscard]] bool isReadable() const noexcept override;

		[[nodiscard]] bool isWriteable() const noexcept override;

		[[nodiscard]] bool isRandomAccess() const noexcept override;

		[[nodiscard]] bool isRandomReadable() const noexcept override;

		[[nodiscard]] bool isRandomWritable() const noexcept override;

	private:
		Arci<IStorage> _backing;

	public:
		[[nodiscard]] Arci<IStorage> getBacking() const noexcept override;
	};
}
