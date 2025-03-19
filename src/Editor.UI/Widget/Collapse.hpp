#pragma once

#include <Engine.Reflow/Children.hpp>
#include <Engine.Reflow.Attributes/Layout/Base/BoxLayoutAttributes.hpp>
#include <Engine.Reflow.Uikit/Atom/Atom.hpp>
#include <Engine.Reflow/Widget/Widget.hpp>

namespace hg::editor::ui {
	class Collapse :
		public engine::reflow::Widget,
		public engine::reflow::uikit::Atom<void, engine::reflow::BoxLayoutAttributes, void> {
	public:
		using this_type = Collapse;

	public:
		Collapse();

		Collapse(mref<engine::reflow::ReflowClassList> classList_, mref<SharedPtr<Widget>> parent_);

		~Collapse() override;

	public:
		[[nodiscard]] string getTag() const noexcept override;

	private:
		bool _collapsed;
		engine::reflow::ReflowEventEmitter::handle_type _toggleHandle;
		engine::reflow::FixedChildren<2uL> _children;

	public:
		void collapse();

		void expand();

	public:
		[[nodiscard]] const ptr<const engine::reflow::FixedChildren<2>> children() const noexcept override;

		void setHeader(mref<SharedPtr<Widget>> header_);

		void setContent(mref<SharedPtr<Widget>> content_);

	public:
		void render(const ptr<engine::reflow::ReflowCommandBuffer> cmd_) override;

	public:
		engine::reflow::PrefetchSizing prefetchSizing(
			engine::reflow::ReflowAxis axis_,
			ref<const engine::reflow::ReflowState> state_
		) const override;

		engine::reflow::PassPrefetchSizing passPrefetchSizing(
			engine::reflow::ReflowAxis axis_,
			ref<const engine::reflow::ReflowPassState> passState_
		) const override;

		math::fvec2 computeReferenceSize(engine::reflow::ReflowAxis axis_) const override;

		void computeSizing(engine::reflow::ReflowAxis axis_, ref<const engine::reflow::ReflowPassState> passState_) override;

		void applyLayout(ref<engine::reflow::ReflowState> state_) override;

		math::fvec2 getGrowFactor() const noexcept override;

		math::fvec2 getShrinkFactor() const noexcept override;
	};
}
