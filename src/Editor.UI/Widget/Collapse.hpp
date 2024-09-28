#pragma once

#include <Engine.Reflow/Widget/Text.hpp>
#include <Engine.Reflow/Widget/HorizontalPanel.hpp>
#include <Engine.Reflow/Widget/VerticalPanel.hpp>
#include <Engine.Reflow/Widget/BoxPanel.hpp>
#include <Engine.Reflow/Children.hpp>
#include <Engine.Reflow/Attribute/Attribute.hpp>
#include <Engine.Reflow/Attribute/DynamicAttribute.hpp>

namespace hg::editor::ui {
	class Collapse;
}

namespace hg::editor::ui {
	class CollapseHeader :
		public engine::reflow::HorizontalPanel {
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
		sptr<engine::reflow::Text> _indicator;
		sptr<engine::reflow::Widget> _content;

	public:
		void setTitle(cref<string> title_);

	public:
		engine::reflow::EventResponse onMouseButtonDown(cref<engine::reflow::MouseEvent> event_) override;

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
		math::vec2 prefetchDesiredSize(cref<engine::reflow::ReflowState> state_, float scale_) const override;

		math::vec2 computeDesiredSize(cref<engine::reflow::ReflowPassState> passState_) const override;

		void applyLayout(ref<engine::reflow::ReflowState> state_, mref<engine::reflow::LayoutContext> ctx_) override;

	public:
		[[nodiscard]] bool willChangeLayout(
			cref<math::vec2> space_
		) const noexcept override;
	};
}
