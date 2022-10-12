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
        void render(const ptr<UICommandBuffer> cmd_) override {}

    protected:
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

    };

}
