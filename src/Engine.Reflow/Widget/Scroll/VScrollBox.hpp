#pragma once
#include "../VerticalPanel.hpp"

namespace hg::engine::reflow {
    class VScrollBox :
        public VerticalPanel {
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
        math::vec2 _scrollValue;
        math::vec2 _overflow;

    public:
        [[nodiscard]] math::vec2 getScrollValue() const noexcept;

        void scrollTo(cref<math::vec2> point_, cref<math::vec2> size_);

    public:
        void render(cref<ReflowState> state_, const ptr<ReflowCommandBuffer> cmd_) override;

    public:
        math::vec2 prefetchDesiredSize(cref<ReflowState> state_, float scale_) const override;

        math::vec2 computeDesiredSize(cref<ReflowPassState> passState_) const override;

        void applyLayout(ref<ReflowState> state_, mref<LayoutContext> ctx_) override;

    public:
        EventResponse onWheel(cref<WheelEvent> event_) override;
    };
}
