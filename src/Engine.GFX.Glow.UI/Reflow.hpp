#pragma once

#include <Engine.Common/Types.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Math/__default.inl>

#include "Command/UICommandBuffer.hpp"
#include "UIContext.hpp"
#include "ReflowType.hpp"
#include "ReflowSpacing.hpp"
#include "ReflowWrapping.hpp"
#include "ReflowOverflow.hpp"

namespace ember::engine::gfx::glow::ui {
    class Widget;
}

namespace ember::engine::gfx::glow::ui {

    struct Reflow final {
        #pragma region Layout
    public:
        /**
         *
         */
        static math::vec2 flow(ptr<Widget> self_, cref<UIContext> ctx_, cref<math::vec2> space_);

    public:
        /**
         *
         */
        static math::vec2 shift(ptr<Widget> self_, cref<UIContext> ctx_, cref<math::vec2> outerOffset_);

        #pragma endregion
        #pragma region Rendering
    public:
        /**
         *
         */
        static void renderContainer(ptr<Widget> self_, const ptr<UICommandBuffer> cmd_);

        #pragma endregion
    };

}
