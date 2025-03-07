#pragma once

#include <Engine.Reflow/Children.hpp>
#include <Engine.Reflow/Widget/Widget.hpp>
#include <Engine.Reflow.Attributes/Layout/Base/BoxLayoutAttributes.hpp>

#include "../../Atom/Atom.hpp"

namespace hg::engine::reflow::uikit {
	class Stack :
		public Widget,
		public Atom<void, BoxLayoutAttributes, void> {
	public:
		using this_type = Stack;

	public:
		Stack();

		~Stack() override;

	public:
		[[nodiscard]] string getTag() const noexcept override;

	protected:
		Children _children;

	public:
		[[nodiscard]] const ptr<const Children> children() const override;

		void addChild(cref<sptr<Widget>> child_);

		void setChild(const u32 idx_, cref<sptr<Widget>> child_);

		void removeChild(cref<sptr<Widget>> child_);

		void clearChildren();

	public:
		void render(const ptr<ReflowCommandBuffer> cmd_) override;

	public:
		void cascadeContextChange(bool invalidate_) override;

		PrefetchSizing prefetchSizing(ReflowAxis axis_, ref<const ReflowState> state_) const override;

		PassPrefetchSizing passPrefetchSizing(ReflowAxis axis_, ref<const ReflowPassState> passState_) const override;

		void computeSizing(ReflowAxis axis_, ref<const ReflowPassState> passState_) override;

		void applyLayout(ref<ReflowState> state_) override;

		[[nodiscard]] math::fvec2 getShrinkFactor() const noexcept override;

		[[nodiscard]] math::fvec2 getGrowFactor() const noexcept override;
	};
}
