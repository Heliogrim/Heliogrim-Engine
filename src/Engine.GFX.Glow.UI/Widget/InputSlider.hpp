#pragma once

#include <concepts>

#include "Input.hpp"

namespace ember::engine::gfx::glow::ui {

    template <typename InputType_>
    concept IsSliderInputType = _STD is_floating_point_v<InputType_> || _STD is_integral_v<InputType_>;

    template <IsSliderInputType InputType_>
    class __declspec(novtable) InputSlider : public Input<InputType_> {
    public:
        using underlying_type = Input<InputType_>;

    public:
        void render(const ptr<UICommandBuffer> cmd_) override {}

    public:
        cref<typename underlying_type::input_type> value() const noexcept override {
            return {};
        }
    };

}
