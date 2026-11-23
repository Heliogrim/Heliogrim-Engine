#pragma once

#include "Atom.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Reflow;
#endif

namespace hg::engine::reflow::uikit {
	class Paint :
		public LeafWidget,
		public Atom<
			void, BoxLayoutAttributes, PaintStyleAttributes
		> {
	public:
		Paint();

		Paint(mref<ReflowClassList> classList_, mref<SharedPtr<Widget>> parent_);

		~Paint() override;

	public:
		[[nodiscard]] string getTag() const noexcept override;

	public:
		void render(const ptr<ReflowCommandBuffer> cmd_) override;

	public:
		void cascadeContextChange(bool invalidate_) override;

		PrefetchSizing prefetchSizing(ReflowAxis axis_, ref<const ReflowState> state_) const override;

		PassPrefetchSizing passPrefetchSizing(ReflowAxis axis_, ref<const ReflowPassState> passState_) const override;

		math::fvec2 getShrinkFactor() const noexcept override;

		math::fvec2 getGrowFactor() const noexcept override;
	};
}
