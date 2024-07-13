#pragma once

#include <Engine.Common/Sal.hpp>
#include <Engine.Common/Collection/AutoArray.hpp>

#include "MemoryArchive.hpp"

namespace hg::engine::resource {
	class MemoryReadWriteArchive :
		public MemoryBaseArchive {
	public:
		using this_type = MemoryReadWriteArchive;

	public:
		MemoryReadWriteArchive(_Inout_ ref<AutoArray<_::byte>> bytes_);

		MemoryReadWriteArchive(_Inout_ ref<AutoArray<_::byte>> bytes_, const s64 limit_);

	protected:
		ref<AutoArray<_::byte>> _bytes;
		s64 _limit;

	public:
		[[nodiscard]] s64 totalSize() const noexcept override;

		[[nodiscard]] bool hasSizeLimit() const noexcept;

		[[nodiscard]] s64 sizeLimit() const noexcept;

		void setSizeLimit(const s64 limit_);

		void serializeBytes(const ptr<void> value_, u64 size_, const ArchiveStreamMode) override;
	};
}
