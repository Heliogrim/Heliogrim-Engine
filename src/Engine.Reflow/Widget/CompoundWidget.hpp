#pragma once

#include "../Children.hpp"
#include "Widget.hpp"

namespace hg::engine::reflow {
	class CompoundWidget :
		public Widget {
	public:
		using this_type = CompoundWidget;

	protected:
		CompoundWidget(mref<ReflowClassList> classList_, mref<SharedPtr<Widget>> parent_);

		CompoundWidget(mref<SharedPtr<Widget>> content_, mref<ReflowClassList> classList_, mref<SharedPtr<Widget>> parent_);

	public:
		~CompoundWidget() override;

	protected:
		SingleChildren _children;

	public:
		[[nodiscard]] const ptr<const SingleChildren> children() const override;

	public:
		void render(const ptr<ReflowCommandBuffer> cmd_) override;

	public:
		PrefetchSizing prefetchSizing(ReflowAxis axis_, ref<const ReflowState> state_) const override;

		PassPrefetchSizing passPrefetchSizing(ReflowAxis axis_, ref<const ReflowPassState> passState_) const override;

		math::fvec2 computeReferenceSize(ReflowAxis axis_) const override;

		void computeSizing(ReflowAxis axis_, ref<const ReflowPassState> passState_) override;

		void applyLayout(ref<ReflowState> state_) override;

		[[nodiscard]] math::fvec2 getShrinkFactor() const noexcept override;

		[[nodiscard]] math::fvec2 getGrowFactor() const noexcept override;
	};
}
