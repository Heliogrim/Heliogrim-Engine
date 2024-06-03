#pragma once

#include <Engine.Common/Types.hpp>

#include "../IStorage.hpp"

namespace hg::engine::storage::system {
	class LocalFileStorage final :
		public IStorage {
	public:
		using this_type = LocalFileStorage;

	public:
		LocalFileStorage() noexcept;

		LocalFileStorage(bool readable_, bool writeable_, bool randomReadable_, bool randomWritable_) noexcept;

		~LocalFileStorage() override = default;

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
	};
}
