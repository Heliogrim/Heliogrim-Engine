#pragma once

#include <Engine.Common/Sal.hpp>

#include "MemoryArchive.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::resource {
	class MemoryWriteonlyArchive :
		public MemoryBaseArchive {
	public:
		using this_type = MemoryWriteonlyArchive;
		using underlying_type = MemoryBaseArchive;

	public:
		MemoryWriteonlyArchive(_Inout_ ref<AutoArray<_::byte>> bytes_);

	protected:
		ref<AutoArray<_::byte>> _bytes;

	public:
		[[nodiscard]] s64 totalSize() const noexcept override;

		void serializeBytes(const ptr<void> value_, u64 size_, const ArchiveStreamMode mode_) override;
	};
}
