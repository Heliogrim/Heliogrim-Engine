#pragma once

#include <Engine.Common/Sal.hpp>
#include <Engine.Common/Concurrent/SharedMemoryReference.hpp>

#include "Language.hpp"
#include "Representation.hpp"
#include "Ir/Module.hpp"

namespace hg::engine::accel::lang {
	class Intermediate {
	public:
		using this_type = Intermediate;

	public:
		Intermediate() noexcept = default;

		explicit Intermediate(cref<this_type> other_);

		Intermediate(mref<this_type> other_) noexcept;

		~Intermediate() = default;

	public:
		Language lang;
		Representation rep;
		/* the resolved module this intermediate was lowered from; shared and never mutated (null for hand-built ones) */
		smr<const ir::Module> ir;

	public:
		/* content hash for variant caching */
		[[nodiscard]] u64 contentHash() const noexcept;

		[[deprecated]] void applyMask(u8 mask_ = 0xFFu);

		void enumerateOutboundSymbols(
			_Out_ ref<Vector<nmpt<const Symbol>>> outbound_,
			u8 mask_ = 0xFFu
		) const noexcept;

		void enumerateInboundSymbols(
			_Out_ ref<Vector<nmpt<const Symbol>>> inbound_,
			u8 mask_ = 0xFFu
		) const noexcept;
	};
}
