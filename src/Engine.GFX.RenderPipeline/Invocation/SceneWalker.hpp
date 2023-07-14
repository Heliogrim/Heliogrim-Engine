#pragma once

#include <Engine.Common/Collection/Vector.hpp>

#include "SceneWalkerFilter.hpp"

namespace hg::engine::gfx::render::pipeline {
    class SceneWalker {
        Vector<SceneWalkerFilter> _filter;
    };
}
