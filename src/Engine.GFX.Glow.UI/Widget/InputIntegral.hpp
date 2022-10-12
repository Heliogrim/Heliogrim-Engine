#pragma once

#include <concepts>
#include <Engine.Common/__macro.hpp>
#include <Engine.Common/Types.hpp>

#include "Input.hpp"

namespace ember::engine::gfx::glow::ui {

    template <std::integral IntegralType_>
    class __declspec(novtable) InputIntegral : public Input<IntegralType_> {};

    class InputInt : public InputIntegral<s32> {
    public:
        void render(const ptr<UICommandBuffer> cmd_) override {}

    public:
        cref<input_type> value() const noexcept override {
            return {};
        }
    };

}
