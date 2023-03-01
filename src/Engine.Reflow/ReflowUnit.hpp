#pragma once

namespace hg::engine::reflow {
    enum class ReflowUnitType {
        eAuto,
        eAbsolute,
        eRelative
    };

    struct ReflowUnit {
        ReflowUnitType type = ReflowUnitType::eAuto;
        float value = 0.F;
    };
}
