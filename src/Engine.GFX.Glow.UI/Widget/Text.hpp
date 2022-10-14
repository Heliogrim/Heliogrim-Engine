#pragma once
#include "Widget.hpp"

namespace ember::engine::gfx::glow::ui {

    class Text : public Widget {
    public:
        using this_type = Text;
        using underlying_type = Widget;

    public:
        Text();

        ~Text() override;

    public:
        void render(const ptr<UICommandBuffer> cmd_) override;

        math::vec2 flow(cref<UIContext> ctx_, cref<math::vec2> available_) override;

        math::vec2 reflow(cref<math::vec2> available_, cref<math::vec2> reflow_) override;

    //private:
    public:
        string _text;
        ptr<Font> _font;

        float _fontSize;
        float _lineHeight;

        u32 _ellipseLines;
    };

}
