#pragma once

#include <Engine.Reflow.Uikit/Atom/Text.hpp>
#include <Engine.Reflow.Uikit/Atom/Layout/HorizontalLayout.hpp>
#include <Engine.Reflow.Uikit/Atom/Layout/VerticalLayout.hpp>

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
		public engine::reflow::uikit::VerticalLayout {
	public:
		friend class ::hg::editor::ui::CollapseHeader;

	public:
		using this_type = Collapse;

	public:
		Collapse();

		~Collapse() override;

	public:
		[[nodiscard]] string getTag() const noexcept override;

	public:
		void setup();

	private:
		bool _collapsed;

	public:
		void collapse();

		void expand();

	protected:
		using VerticalLayout::addChild;
		using VerticalLayout::setChild;
		using VerticalLayout::removeChild;

	public:
		[[nodiscard]] sptr<CollapseHeader> getHeader() noexcept;

		[[nodiscard]] sptr<Widget> getContent() const noexcept;

		void setContent(cref<sptr<Widget>> widget_);

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
