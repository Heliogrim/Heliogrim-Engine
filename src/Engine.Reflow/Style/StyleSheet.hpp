#pragma once

#include <Engine.Common/Math/Vector.hpp>
#include <Engine.GFX/Color/Color.hpp>

#include "StyleSheetEntry.hpp"
#include "../BorderRadius.hpp"
#include "../Margin.hpp"
#include "../Padding.hpp"
#include "../ReflowType.hpp"
#include "../ReflowSpacing.hpp"
#include "../ReflowUnit.hpp"
#include "../ReflowWrap.hpp"
#include "../TextAlign.hpp"

namespace ember::engine::reflow {
    class Font;
}

namespace ember::engine::reflow {
    struct StyleSheet {
        StyleSheetEntry<ReflowUnit> minWidth { { ReflowUnitType::eAuto, 0.F } };
        StyleSheetEntry<ReflowUnit> width { { ReflowUnitType::eAuto, 0.F } };
        StyleSheetEntry<ReflowUnit> maxWidth { { ReflowUnitType::eAuto, 0.F } };

        StyleSheetEntry<ReflowUnit> minHeight { { ReflowUnitType::eAuto, 0.F } };
        StyleSheetEntry<ReflowUnit> height { { ReflowUnitType::eAuto, 0.F } };
        StyleSheetEntry<ReflowUnit> maxHeight { { ReflowUnitType::eAuto, 0.F } };

        StyleSheetEntry<ReflowWrap> wrap { ReflowWrap::eNone };
        StyleSheetEntry<ReflowUnit> colGap { { ReflowUnitType::eAuto, 0.F } };
        StyleSheetEntry<ReflowUnit> rowGap { { ReflowUnitType::eAuto, 0.F } };

        StyleSheetEntry<Padding> padding { { 0.F, 0.F, 0.F, 0.F } };
        StyleSheetEntry<Margin> margin { { 0.F, 0.F, 0.F, 0.F } };

        StyleSheetEntry<ReflowSpacing> reflowSpacing { ReflowSpacing::eStart };
        StyleSheetEntry<float> reflowShrink { 0.F };
        StyleSheetEntry<float> reflowGrow { 0.F };

        StyleSheetEntry<Padding> border { { 0.F, 0.F, 0.F, 0.F } };
        StyleSheetEntry<BorderRadius> borderRadius { { 0.F, 0.F, 0.F, 0.F } };
        StyleSheetEntry<engine::color> borderColor { engine::color { 255.F, 255.F, 255.F, 255.F } };

        StyleSheetEntry<engine::color> color { engine::color { 255.F, 255.F, 255.F, 255.F } };

        StyleSheetEntry<ptr<Font>> font { nullptr };
        StyleSheetEntry<float> fontSize { 16.F };
        StyleSheetEntry<float> lineHeight { 1.F };
        StyleSheetEntry<TextAlign> textAlign { TextAlign::eLeftMiddle };
        StyleSheetEntry<u32> textEllipse { 0ui32 };
    };
}
