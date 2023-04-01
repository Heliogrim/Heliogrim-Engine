#pragma once
#include <Engine.GFX/Color/Color.hpp>

namespace hg::editor::ui::color {
    struct Dark {
        inline static const engine::color backgroundDefault { 46.F, 50.F, 58.F, 255.F };
        inline static const engine::color backgroundInnerField { 40.F, 44.F, 52.F, 255.F };
        inline static const engine::color backgroundInnerFieldDarken { 32.F, 36.F, 44.F, 255.F };

        inline static const engine::color raisedColor { 53.F, 57.F, 65.F, 255.F };

        inline static const engine::color backgroundText { 77.F, 81.F, 89.F, 255.F };
        inline static const engine::color backgroundRaisedText { 77.F, 127.F, 127.F, 255.F };

        inline static const engine::color darkGreen { 0.F, 100.F, 0.F, 255.F };
        inline static const engine::color darkRed { 139.F, 0.F, 0.F, 255.F };

        inline static const engine::color white { 255.F, 255.F, 255.F, 255.F };
        inline static const engine::color grey { 127.F, 127.F, 127.F, 255.F };
        inline static const engine::color black { 0.F, 0.F, 0.F, 255.F };
        inline static const engine::color transparent { 0.F, 0.F, 0.F, 0.F };
    };
}
