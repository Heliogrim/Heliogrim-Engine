#pragma once

#include <Engine.Reflow/Children.hpp>
#include <Engine.Reflow/Widget/Widget.hpp>
#include <Engine.Reflow.Attributes/Style/Base/PaintStyleAttributes.hpp>

#include "Atom.hpp"

namespace hg::engine::reflow::uikit {
	class Paint :
		public Widget,
		public Atom<
			void, void, PaintStyleAttributes
		> {
	public:
		Paint();

		Paint(mref<ReflowClassList> classList_, mref<SharedPtr<Widget>> parent_);

		~Paint() override;

	public:
		[[nodiscard]] string getTag() const noexcept override;

	protected:
		SingleChildren _children;

	public:
		const ptr<const SingleChildren> children() const override;

		void setChild(cref<SharedPtr<Widget>> nextChild_);

	public:
		void render(const ptr<ReflowCommandBuffer> cmd_) override;

	public:
		void cascadeContextChange(bool invalidate_) override;

		math::vec2 prefetchDesiredSize(cref<ReflowState> state_, float scale_) const override;

		math::vec2 computeDesiredSize(cref<ReflowPassState> passState_) const override;

		void applyLayout(ref<ReflowState> state_, mref<LayoutContext> ctx_) override;
	};
}
