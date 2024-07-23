#pragma once
#include <Engine.Common/String.hpp>

#include "IStorageRepository.hpp"

namespace hg::engine::storage::system {
	class macro_novtable IStorageFileRepository :
		public IStorageRepository {
	public:
		using this_type = IStorageFileRepository;

	protected:
		constexpr IStorageFileRepository() noexcept = default;

	public:
		IStorageFileRepository(cref<this_type>) = delete;

		IStorageFileRepository(mref<this_type>) = delete;

		constexpr ~IStorageFileRepository() override = default;

	public:
		/**
		 *
		 * @note Every file repository should encode exactly one unique pair of the storage class and
		 *		a single mountpoint. A string view with an underlying nullptr and an empty string will be interpreted
		 *		as root mounted.
		 * @returns The mountpoint of this repository within the virtual filesystem
		 */
		[[nodiscard]] virtual StringView getVfsMountPoint() const noexcept = 0;
	};
}
