#pragma once

#include "Widget.hpp"

namespace hg::engine::reflow {
	class NullWidget final :
		public Widget {
	public:
		using this_type = NullWidget;

	protected:
		NullWidget();

	public:
		~NullWidget() override;

	public:
		static sptr<NullWidget> instance();

	public:
		[[nodiscard]] string getTag() const noexcept override;

	public:
		[[nodiscard]] const ptr<const Children> children() const override;

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
