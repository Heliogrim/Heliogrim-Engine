#pragma once

#include <Engine.Common/__macro.hpp>

#include "StageIdentifier.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::scheduler {
	class Stage final {
	public:
		using this_type = Stage;

	public:
		Stage(mref<StageIdentifier> identifier_, const s8 slot_) noexcept;

		Stage (cref<this_type>) = delete;

		Stage(mref<this_type>) noexcept = delete;

		~Stage() noexcept = default;

	private:
		u16 _refCount;

	public:
		void incRef() noexcept;

		[[nodiscard]] bool decRef() noexcept;

	private:
		const StageIdentifier _identifier;
		const s8 _slot;

	public:
		[[nodiscard]] cref<StageIdentifier> getIdentifier() const noexcept;

		[[nodiscard]] s8 getSlot() const noexcept;
	};
}