#pragma once

#include <Engine.Common/Collection/DenseMap.hpp>

#include "ReflowPassState.hpp"
#include "Engine.Common/Memory/MemoryPointer.hpp"
#include "Widget/__fwd.hpp"

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
		u16 _renderTick = 0u;
		DenseMap<ptr<void>, non_owning_rptr<ReflowPassState>> _recorded;
		u32 _layoutCalls = 0u;

	public:
		[[nodiscard]] u16 getRenderTick() const noexcept;

		void setRenderTick(u16 renderTick_) noexcept;

		[[nodiscard]] nmpt<ReflowPassState> record(cref<sptr<Widget>> widget_);

		[[nodiscard]] nmpt<ReflowPassState> record(nmpt<Widget> widget_);

		[[nodiscard]] bool isRecorded(nmpt<const Widget> widget_) const noexcept;

		[[nodiscard]] nmpt<const ReflowPassState> getStateOf(cref<sptr<Widget>> widget_) const;

		[[nodiscard]] nmpt<const ReflowPassState> getStateOf(nmpt<const Widget> widget_) const;

		[[nodiscard]] nmpt<ReflowPassState> getStateOf(cref<sptr<Widget>> widget_);

		[[nodiscard]] nmpt<ReflowPassState> getStateOf(nmpt<Widget> widget_);

	public:
		/* Number of prefetchSizing, computeSizing and applyLayout calls; reset() keeps it */
		[[nodiscard]] u32 layoutCalls() const noexcept;

		void countLayoutCall() noexcept;
	};
}
