#pragma once
#include "Input.hpp"

namespace ember::engine::gfx::glow::ui {

    template <typename InputType_>
    class __declspec(novtable) InputSelect : public Input<InputType_> {
    public:
        using underlying_type = Input<InputType_>;

    public:
        void render(const ptr<UICommandBuffer> cmd_) override {}

    protected:
        typename underlying_type::input_type _selected;

    public:
        cref<typename underlying_type::input_type> value() const noexcept override {
            return _selected;
        }
    };

}
