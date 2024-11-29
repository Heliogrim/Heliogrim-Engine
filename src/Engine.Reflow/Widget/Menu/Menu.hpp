#pragma once

#include "../Widget.hpp"
#include "../../Children.hpp"
#include "Engine.Event/EventEmitter.hpp"

namespace hg::engine::reflow {
	class Menu final :
		public Widget {
	public:
		using this_type = Menu;

	public:
		Menu();

		~Menu() override;

	public:
		[[nodiscard]] string getTag() const noexcept override;

	public:
		void openMenu();

		void closeMenu();

	private:
		EventEmitter::handle_type _triggerHandle;
		SingleChildren _trigger;
		sptr<Anchor> _anchor;
		sptr<Widget> _content;

	public:
		[[nodiscard]] const ptr<const SingleChildren> children() const override;

		[[nodiscard]] sptr<Widget> getTrigger() const noexcept;

		[[nodiscard]] sptr<Widget> getContent() const noexcept;

		void setTrigger(mref<sptr<Button>> trigger_) noexcept;

		void setContent(mref<sptr<Widget>> content_) noexcept;

	public:
		void render(ptr<ReflowCommandBuffer> cmd_) override;

	public:
		math::vec2 prefetchDesiredSize(cref<ReflowState> state_, float scale_) const override;

		math::vec2 computeDesiredSize(cref<ReflowPassState> passState_) const override;

		void applyLayout(ref<ReflowState> state_, mref<LayoutContext> ctx_) override;

	public:
		EventResponse onFocus(cref<FocusEvent> event_) override;

		EventResponse onBlur(cref<FocusEvent> event_) override;
	};
}
