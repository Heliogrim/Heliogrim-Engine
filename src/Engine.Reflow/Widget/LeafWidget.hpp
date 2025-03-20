#pragma once

#include "Widget.hpp"
#include "../Children.hpp"

namespace hg::engine::reflow {
	class LeafWidget :
		public Widget {
	public:
		using this_type = LeafWidget;

	protected:
		LeafWidget(mref<ReflowClassList> classList_, mref<SharedPtr<Widget>> parent_);

	public:
		~LeafWidget() override;

	public:
		[[nodiscard]] string getTag() const noexcept override = 0;

	public:
		[[nodiscard]] const ptr<const NullChildren> children() const override;

	public:
		void render(const ptr<ReflowCommandBuffer> cmd_) override = 0;

	public:
		PrefetchSizing prefetchSizing(ReflowAxis axis_, ref<const ReflowState> state_) const override = 0;

		PassPrefetchSizing passPrefetchSizing(ReflowAxis axis_, ref<const ReflowPassState> passState_) const override = 0;

		math::fvec2 computeReferenceSize(ReflowAxis axis_) const override;

		void computeSizing(ReflowAxis axis_, ref<const ReflowPassState> passState_) override;

		void applyLayout(ref<ReflowState> state_) override;

		[[nodiscard]] math::fvec2 getShrinkFactor() const noexcept override = 0;

		[[nodiscard]] math::fvec2 getGrowFactor() const noexcept override = 0;
	};
}
