#pragma once

#include <Engine.Reflow/Children.hpp>
#include <Engine.Reflow.Uikit/Atom/Text.hpp>
#include <Engine.Reflow/Attribute/Attribute.hpp>
#include <Engine.Reflow/Attribute/DynamicAttribute.hpp>
#include <Engine.Reflow.Uikit/Atom/Layout/HorizontalLayout.hpp>

namespace hg::editor::ui {
	class Collapse;
}

namespace hg::editor::ui {
	class CollapseHeader :
		public engine::reflow::uikit::HorizontalLayout {
	public:
		friend class ::hg::editor::ui::Collapse;

	public:
		using this_type = CollapseHeader;

	public:
		CollapseHeader(ptr<Collapse> parent_);

		~CollapseHeader() override;

	private:
		void setup();

	private:
		ptr<Collapse> _parent;

	private:
		sptr<engine::reflow::uikit::Text> _indicator;
		sptr<engine::reflow::Widget> _content;

	public:
		void setTitle(cref<string> title_);

	public:
		engine::reflow::EventResponse invokeOnMouseButtonDown(ref<const engine::reflow::MouseEvent> event_) override;

	public:
		[[nodiscard]] string getTag() const noexcept override;
	};

	class Collapse :
		public engine::reflow::Widget {
	public:
		friend class ::hg::editor::ui::CollapseHeader;

	public:
		using this_type = Collapse;

	public:
		Collapse();

		~Collapse() override;

	public:
		void setup();

	private:
	public:
		struct Attributes {
			engine::reflow::Attribute<engine::reflow::ReflowUnit> minWidth;
			engine::reflow::Attribute<engine::reflow::ReflowUnit> maxWidth;

			engine::reflow::Attribute<engine::reflow::ReflowUnit> minHeight;
			engine::reflow::Attribute<engine::reflow::ReflowUnit> maxHeight;

			engine::reflow::DynamicAttribute<bool> collapsed;
		} attr;

	public:
		void collapse();

		void expand();

	private:
		engine::reflow::FixedChildren<2> _children;

	public:
		[[nodiscard]] const ptr<const engine::reflow::Children> children() const override;

		[[nodiscard]] sptr<CollapseHeader> getHeader() noexcept;

		[[nodiscard]] sptr<Widget> getContent() const noexcept;

		void setContent(cref<sptr<Widget>> widget_);

	public:
		[[nodiscard]] string getTag() const noexcept override;

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

		void computeSizing(engine::reflow::ReflowAxis axis_, ref<const engine::reflow::ReflowPassState> passState_) override;

		void applyLayout(ref<engine::reflow::ReflowState> state_) override;

		math::fvec2 getGrowFactor() const noexcept override;

		math::fvec2 getShrinkFactor() const noexcept override;
	};
}
