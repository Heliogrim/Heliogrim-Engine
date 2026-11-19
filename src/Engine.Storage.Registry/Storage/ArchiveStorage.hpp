#pragma once

#include <Engine.Common/__macro.hpp>
#include <Engine.Resource.Archive/ArchiveGuid.hpp>

#include "../IStorage.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::storage::system {
	class ArchiveStorage final :
		public InheritMeta<ArchiveStorage, ISecondaryStorage> {
	public:
		using this_type = ArchiveStorage;

	public:
		ArchiveStorage() = delete;

		ArchiveStorage(
			mref<Arci<IStorage>> backing_,
			mref<resource::ArchiveGuid> archiveGuid_,
			bool readable_,
			bool writeable_,
			bool randomReadable_,
			bool randomWritable_
		) noexcept;

		~ArchiveStorage() override = default;

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
		resource::ArchiveGuid _archiveGuid;

	public:
		[[nodiscard]] Arci<IStorage> getBacking() const noexcept override;

		[[nodiscard]] resource::ArchiveGuid getArchiveGuid() const noexcept;
	};
}
