#pragma once

#include <Engine.Common/Collection/DenseMap.hpp>

#include "Widget/__fwd.hpp"
#include "Widget/WidgetState.hpp"
#include "ReflowPassState.hpp"
#include "Engine.Common/Memory/MemoryPointer.hpp"

namespace hg::engine::reflow {
	class ReflowState {
	public:
		using this_type = ReflowState;

	public:
		ReflowState();

		~ReflowState();

	public:
		void reset();

	private:
		u16 _renderTick;
		DenseMap<ptr<void>, non_owning_rptr<ReflowPassState>> _recorded;

	public:
		[[nodiscard]] u16 getRenderTick() const noexcept;

		void setRenderTick(u16 renderTick_) noexcept;

		[[nodiscard]] nmpt<ReflowPassState> record(cref<sptr<Widget>> widget_);

		[[nodiscard]] nmpt<const ReflowPassState> getStateOf(cref<sptr<Widget>> widget_) const;

		[[nodiscard]] nmpt<const ReflowPassState> getStateOf(
			nmpt<const Widget> widget_
		) const;

		[[nodiscard]] nmpt<ReflowPassState> getStateOf(cref<sptr<Widget>> widget_);
	};
}
