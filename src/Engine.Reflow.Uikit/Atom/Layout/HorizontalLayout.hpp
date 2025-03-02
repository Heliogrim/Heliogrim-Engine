#pragma once

#include <Engine.Reflow/Children.hpp>
#include <Engine.Reflow.Attributes/Layout/CompositeLayoutAttributes.hpp>
#include <Engine.Reflow/Widget/Widget.hpp>
#include <Engine.Reflow.Attributes/Layout/Base/BoxLayoutAttributes.hpp>
#include <Engine.Reflow.Attributes/Layout/Base/FlexLayoutAttributes.hpp>

#include "../Atom.hpp"

namespace hg::engine::reflow::uikit {
	class HorizontalLayout :
		public Widget,
		public Atom<void, CompositeLayoutAttributes<BoxLayoutAttributes, FlexLayoutAttributes>, void> {
	public:
		using this_type = HorizontalLayout;

	public:
		HorizontalLayout();

		HorizontalLayout(mref<ReflowClassList> classList_, mref<SharedPtr<Widget>> parent_);

		~HorizontalLayout() override;

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
		math::vec2 prefetchDesiredSize(cref<ReflowState> state_, float scale_) const override;

		math::vec2 computeDesiredSize(cref<ReflowPassState> passState_) const override;

		void applyLayout(ref<ReflowState> state_, mref<LayoutContext> ctx_) override;

	public:
		[[nodiscard]] float shrinkFactor() const noexcept override;

		[[nodiscard]] float growFactor() const noexcept override;
	};
}
