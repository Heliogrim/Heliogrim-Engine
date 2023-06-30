#pragma once

#include <Engine.Common/String.hpp>
#include <Engine.Common/Collection/Vector.hpp>

#include "ModuleTargetStage.hpp"

namespace hg::engine::gfx::acc {
    struct ModuleSource {
        ModuleTargetStage targetStage;
        Vector<string> sourceCode;
    };
}
