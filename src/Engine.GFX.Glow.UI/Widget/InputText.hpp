#pragma once

#include <Engine.Common/String.hpp>

#include "Input.hpp"

namespace ember::engine::gfx::glow::ui {

    class InputText : public Input<string> {
    public:
        using this_type = InputText;

    public:
        InputText();

        ~InputText() override = default;

    public:
        void render(const ptr<UICommandBuffer> cmd_) override;

    public:
        [[nodiscard]] engine::color statedColor() const noexcept override;

    private:
        [[nodiscard]] math::vec2 baseFlowSize() const;

    public:
        math::vec2 flow(cref<UIContext> ctx_, cref<math::vec2> available_) override;

    protected:
        math::vec2 _contentSize;
        string _value;

    public:
        cref<input_type> value() const noexcept override {
            return _value;
        }

    private:
        string _placeholder;

    public:
        [[nodiscard]] cref<string> placeholder() const noexcept;

        void setPlaceholder(cref<string> text_);

        //private:
    public:
        ptr<Font> _font;
        float _fontSize;
        float _lineHeight;
        u32 _ellipseLines;

        //private:
    public:
        u8 _textAlign;

        //private:
    public:
        color _baseColor;
        color _baseBackground;
        color _raisedColor;
        color _raisedBackground;
        color _focusColor;
        color _focusBackground;

    public:
        bool onMouseButtonEvent(cref<math::ivec2> pointer_, u32 button_, bool down_, u32 modifier_) override;
    };

}
