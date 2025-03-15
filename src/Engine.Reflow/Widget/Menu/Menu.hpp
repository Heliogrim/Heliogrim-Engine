#pragma once

#include "../Widget.hpp"
#include "../../Children.hpp"

namespace hg::engine::reflow {
	enum class MenuContentPosition {
		eTop = 0b1 << 0,
		eBottom = 0b1 << 1,
		eVCenter = eTop | eBottom,
		/**/
		eLeft = 0b1 << 2,
		eRight = 0b1 << 3,
		eHCenter = eLeft | eRight,
		/**/
		eTopLeft = eTop | eLeft,
		eTopCenter = eTop | eHCenter,
		eTopRight = eTop | eRight,
		/**/
		eVCenterLeft = eVCenter | eLeft,
		eVCenterRight = eVCenter | eRight,
		/**/
		eBottomLeft = eBottom | eLeft,
		eBottomCenter = eBottom | eHCenter,
		eBottomRight = eBottom | eRight,
		/**/
		eCenterCenter = eVCenter | eHCenter
	};

	/**/

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
		ReflowEventEmitter::handle_type _triggerHandle;
		SingleChildren _trigger;
		sptr<Anchor> _anchor;
		sptr<Widget> _content;

		MenuContentPosition _contentPosition;

	public:
		[[nodiscard]] const ptr<const SingleChildren> children() const override;

		[[nodiscard]] sptr<Widget> getTrigger() const noexcept;

		[[nodiscard]] sptr<Widget> getContent() const noexcept;

		void setTrigger(mref<SharedPtr<Widget>> trigger_) noexcept;

		void setContent(mref<sptr<Widget>> content_) noexcept;

		void setContentPosition(MenuContentPosition contentPosition_) noexcept;

	public:
		void render(ptr<ReflowCommandBuffer> cmd_) override;

	public:
		PrefetchSizing prefetchSizing(ReflowAxis axis_, ref<const ReflowState> state_) const override;

		PassPrefetchSizing passPrefetchSizing(ReflowAxis axis_, ref<const ReflowPassState> passState_) const override;

		math::fvec2 computeReferenceSize(ReflowAxis axis_) const override;

		void computeSizing(ReflowAxis axis_, ref<const ReflowPassState> passState_) override;

		void applyLayout(ref<ReflowState> state_) override;

		math::fvec2 getGrowFactor() const noexcept override;

		math::fvec2 getShrinkFactor() const noexcept override;

		math::vec2 computeContentPosition();

	public:
		EventResponse invokeOnFocus(cref<FocusEvent> event_) override;

		EventResponse invokeOnBlur(cref<FocusEvent> event_) override;
	};
}
