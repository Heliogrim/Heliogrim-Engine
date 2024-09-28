#pragma once

#include <Engine.Common/Wrapper.hpp>

#include "__fwd.hpp"

namespace hg::engine::serialization {
	enum class ScopedSlotGuardMode : bool {
		eRead = false,
		eWrite = true
	};

	/**
	 * RAII Guard for `complete` mutating slot operations
	 */
	class ScopedSlotGuard final {
	public:
		using this_type = ScopedSlotGuard;

	public:
		explicit ScopedSlotGuard(
			const non_owning_rptr<const ScopedSlot> scopedSlot_,
			const ScopedSlotGuardMode mode_
		);

		ScopedSlotGuard(cref<this_type>) = delete;

		ScopedSlotGuard(mref<this_type>) noexcept = delete;

		~ScopedSlotGuard();

	private:
		const ScopedSlotGuardMode _mode;
		const non_owning_rptr<const ScopedSlot> _scopedSlot;
	};
}
