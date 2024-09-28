#pragma once

#include <Engine.Common/Sal.hpp>

#include "ReflowState.hpp"
#include "Layout/LayoutContext.hpp"
#include "Widget/__fwd.hpp"

namespace hg::engine::reflow {
	class ReflowEngine {
	private:
		static std::atomic_uint_fast16_t _globalReflowTick;

	public:
		static void updateTickVersion() noexcept;

		[[nodiscard]] static u16 getGlobalRenderTick() noexcept;

	public:
		static void tick(
			_Inout_ ref<ReflowState> state_,
			_In_ cref<sptr<Widget>> widget_,
			_In_ mref<LayoutContext> globalCtx_
		);
	};
}
