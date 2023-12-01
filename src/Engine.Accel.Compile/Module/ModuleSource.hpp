#pragma once

#include <Engine.Accel.Lang/Language.hpp>

#include "ModuleTargetStage.hpp"

namespace hg::engine::accel {
    struct ModuleSource {
        ModuleTargetStage targetStage;
        lang::Language code;
    };
}
