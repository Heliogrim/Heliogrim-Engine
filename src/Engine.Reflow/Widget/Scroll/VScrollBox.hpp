#pragma once
#include "../VBox.hpp"

namespace hg::engine::reflow {
    class VScrollBox :
        public VBox {
    public:
        using this_type = VScrollBox;

    public:
        VScrollBox(mref<sptr<BoundStyleSheet>> style_);

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
        void render(const ptr<ReflowCommandBuffer> cmd_) override;

        void flow(
            cref<FlowContext> ctx_,
            cref<math::vec2> space_,
            cref<math::vec2> limit_,
            ref<StyleKeyStack> styleStack_
        ) override;

        void shift(cref<FlowContext> ctx_, cref<math::vec2> offset_) override;

    private:
        [[nodiscard]] math::vec2 contentSize() const noexcept;

    public:
        EventResponse onWheel(cref<WheelEvent> event_) override;
    };
}
