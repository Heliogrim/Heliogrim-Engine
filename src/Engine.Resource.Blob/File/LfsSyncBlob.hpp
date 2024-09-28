#pragma once

#include <fstream>
#include <span>
#include <Engine.Common/Types.hpp>
#include <Engine.Common/UniqueValue.hpp>
#include <Engine.Filesystem/Url.hpp>

#include "../Blob.hpp"

namespace hg::engine::resource {
	// TODO: Determine how we should handle the difference between 'readWrite' and 'readonly' access.
	class LfsSyncBlob final :
		public Blob {
	public:
		explicit LfsSyncBlob(mref<fs::Path> path_) noexcept;

		LfsSyncBlob(cref<LfsSyncBlob>) = delete;

		constexpr LfsSyncBlob(mref<LfsSyncBlob>) noexcept = default;

		constexpr ~LfsSyncBlob() override = default;

	private:
		UniqueValue<std::fstream> _stream;

	public:
		std::span<_::byte> read(streamoff offset_, mref<std::span<_::byte>> dst_) const override;

		std::span<_::byte> write(streamoff offset_, mref<std::span<_::byte>> src_) override;

	public:
		[[nodiscard]] streamsize size() const noexcept override;
	};
}
