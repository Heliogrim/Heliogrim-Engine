#pragma once

#include <Engine.Reflow/Children.hpp>
#include <Engine.Reflow/Widget/Widget.hpp>
#include <Engine.Reflow.Attributes/Layout/Base/BoxLayoutAttributes.hpp>

#include "../Atom.hpp"

namespace hg::engine::reflow::uikit {
	class BoxLayout :
		public Widget,
		public Atom<void, BoxLayoutAttributes, void> {
	public:
		using this_type = BoxLayout;

	public:
		BoxLayout();

		BoxLayout(mref<ReflowClassList> classList_, mref<SharedPtr<Widget>> parent_);

		~BoxLayout() override;

	public:
		string getTag() const noexcept override;

	protected:
		SingleChildren _children;

	public:
		[[nodiscard]] const ptr<const Children> children() const override;

		void setChild(cref<sptr<Widget>> nextChild_);

	public:
		void render(const ptr<ReflowCommandBuffer> cmd_) override;

	public:
		math::vec2 prefetchDesiredSize(cref<ReflowState> state_, float scale_) const override;

		math::vec2 computeDesiredSize(cref<ReflowPassState> passState_) const override;

		void applyLayout(ref<ReflowState> state_, mref<LayoutContext> ctx_) override;
	};
}
