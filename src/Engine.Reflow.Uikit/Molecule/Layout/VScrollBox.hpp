#pragma once

#include "../../Atom/Layout/VerticalLayout.hpp"

namespace hg::engine::reflow::uikit {
	class VScrollBox :
		public VerticalLayout {
	public:
		using this_type = VScrollBox;

	public:
		VScrollBox();

		~VScrollBox() override;

	public:
		[[nodiscard]] string getTag() const noexcept override;

	private:
		sptr<Widget> _scrollTrack;
		sptr<Widget> _scrollThumb;

	public:
		void setScrollTrack(cref<sptr<Widget>> track_);

		void setScrollThumb(cref<sptr<Widget>> thumb_);

	private:
		f32 _scrollValue;
		f32 _overflow;

	public:
		[[nodiscard]] f32 getScrollValue() const noexcept;

		void scrollTo(cref<math::vec2> point_, cref<math::vec2> size_);

	public:
		void render(const ptr<ReflowCommandBuffer> cmd_) override;

	public:
		PrefetchSizing prefetchSizing(ReflowAxis axis_, ref<const ReflowState> state_) const override;

		void computeSizing(ReflowAxis axis_, ref<const ReflowPassState> passState_) override;

		void applyLayout(ref<ReflowState> state_) override;

	public:
		EventResponse invokeOnWheel(ref<const WheelEvent> event_) override;
	};
}
