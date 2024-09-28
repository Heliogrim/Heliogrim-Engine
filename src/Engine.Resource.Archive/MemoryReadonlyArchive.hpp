#pragma once

#include <span>
#include <Engine.Common/Sal.hpp>

#include "MemoryArchive.hpp"

namespace hg::engine::resource {
	class MemoryReadonlyArchive :
		public MemoryBaseArchive {
	public:
		using this_type = MemoryReadonlyArchive;
		using underlying_type = MemoryBaseArchive;

	public:
		MemoryReadonlyArchive(_In_ ref<std::span<u8>> bytes_);

		MemoryReadonlyArchive(_In_ ref<std::span<u8>> bytes_, const s64 limit_);

	protected:
		ref<std::span<u8>> _bytes;
		s64 _limit;

	public:
		[[nodiscard]] s64 totalSize() const noexcept override;

		[[nodiscard]] bool hasSizeLimit() const noexcept;

		[[nodiscard]] s64 sizeLimit() const noexcept;

		void setSizeLimit(const s64 limit_);

		void serializeBytes(const ptr<void> value_, u64 size_, const ArchiveStreamMode mode_) override;
	};
}
