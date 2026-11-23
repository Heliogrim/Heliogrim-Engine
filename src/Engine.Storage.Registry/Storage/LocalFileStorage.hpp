#pragma once

#include "../IStorage.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
import Engine.Filesystem;
#endif

namespace hg::engine::storage {
	class StorageIo;
}

namespace hg::engine::storage::system {
	class LocalFileStorage final :
		public InheritMeta<LocalFileStorage, IStorage> {
	public:
		using this_type = LocalFileStorage;

	public:
		LocalFileStorage() = delete;

		LocalFileStorage(
			mref<fs::Path> lfsPath_,
			bool readable_,
			bool writeable_,
			bool randomReadable_,
			bool randomWritable_
		) noexcept;

		~LocalFileStorage() override = default;

	public:
		[[nodiscard]] constexpr bool isPrimaryStorage() const noexcept override {
			return true;
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
	public:
		fs::Path _lfsPath;
	};
}
