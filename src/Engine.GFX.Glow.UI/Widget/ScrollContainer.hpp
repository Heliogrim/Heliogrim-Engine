#pragma once
#include "ReflowContainer.hpp"

namespace ember::engine::gfx::glow::ui {

    class ScrollContainer : public ReflowContainer {
    public:
        ScrollContainer();

        ~ScrollContainer() override = default;

    public:
        void render(const ptr<UICommandBuffer> cmd_) override;

        void shift(cref<UIContext> ctx_, cref<math::vec2> offset_) override;

    public:
        bool onScrollEvent(cref<math::ivec2> pointer_, cref<math::vec2> value_) override;

        //private:
    public:
        math::vec2 _scrollValue;

        //private:
    public:
        engine::color _scrollTrackColor;
        engine::color _scrollThumbColor;
    };
}
