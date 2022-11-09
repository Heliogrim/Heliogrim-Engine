#pragma once
#include "../VBox.hpp"

namespace ember::engine::reflow {

    class VScrollBox :
        public VBox {
    public:
        using this_type = VScrollBox;

    public:
        VScrollBox(mref<sptr<BoundStyleSheet>> style_);

        ~VScrollBox() override;

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
        void render(const ptr<ReflowCommandBuffer> cmd_) override;

        void flow(cref<FlowContext> ctx_, cref<math::vec2> space_, ref<StyleKeyStack> styleStack_) override;

        void shift(cref<FlowContext> ctx_, cref<math::vec2> offset_) override;

    private:
        [[nodiscard]] math::vec2 contentSize() const noexcept;

    public:
        EventResponse onWheel(cref<WheelEvent> event_) override;
    };

}
