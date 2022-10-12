#pragma once
#include "Input.hpp"

namespace ember::engine::gfx::glow::ui {

    class InputFloat : public Input<float> {
    public:
        void render(const ptr<UICommandBuffer> cmd_) override {}

    public:
        cref<input_type> value() const noexcept override {
            return {};
        }
    };

}
