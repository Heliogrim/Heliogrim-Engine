#pragma once

#include "../Atom.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Reflow;
#endif

namespace hg::engine::reflow::uikit {
	class VerticalLayout :
		public Widget,
		public Atom<void, CompositeLayoutAttributes<BoxLayoutAttributes, FlexLayoutAttributes>, void> {
	public:
		using this_type = VerticalLayout;

	public:
		VerticalLayout();

		VerticalLayout(mref<ReflowClassList> classList_, mref<SharedPtr<Widget>> parent_);

		~VerticalLayout() override;

	public:
		[[nodiscard]] string getTag() const noexcept override;

	protected:
		Children _children;

	public:
		const ptr<const Children> children() const override;

		void addChild(cref<sptr<Widget>> child_);

		void setChild(const u32 idx_, cref<sptr<Widget>> child_);

		void removeChild(cref<sptr<Widget>> child_);

		void clearChildren();

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
