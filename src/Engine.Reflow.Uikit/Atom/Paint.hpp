#pragma once

#include <Engine.Reflow/Children.hpp>
#include <Engine.Reflow/Widget/Widget.hpp>
#include <Engine.Reflow.Attributes/Layout/Base/BoxLayoutAttributes.hpp>
#include <Engine.Reflow.Attributes/Style/Base/PaintStyle.hpp>

#include "Atom.hpp"

namespace hg::engine::reflow::uikit {
	class Paint :
		public Widget,
		public Atom<
			void, BoxLayoutAttributes, PaintStyleAttributes
		> {
	public:
		Paint();

		Paint(mref<ReflowClassList> classList_, mref<SharedPtr<Widget>> parent_);

		~Paint() override;

	public:
		[[nodiscard]] string getTag() const noexcept override;

	protected:
		NullChildren _children;

	public:
		const ptr<const NullChildren> children() const override;

	public:
		void render(const ptr<ReflowCommandBuffer> cmd_) override;

	public:
		void cascadeContextChange(bool invalidate_) override;

		PrefetchSizing prefetchSizing(ReflowAxis axis_, ref<const ReflowState> state_) const override;

		PassPrefetchSizing passPrefetchSizing(ReflowAxis axis_, ref<const ReflowPassState> passState_) const override;

		math::fvec2 computeReferenceSize(ReflowAxis axis_) const override;

		void computeSizing(ReflowAxis axis_, ref<const ReflowPassState> passState_) override;

		void applyLayout(ref<ReflowState> state_) override;

		math::fvec2 getShrinkFactor() const noexcept override;

		math::fvec2 getGrowFactor() const noexcept override;
	};
}
