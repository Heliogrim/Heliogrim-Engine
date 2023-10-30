#pragma once
#include "../SceneWalkerFilter.hpp"

namespace hg::engine::gfx::render::graph {
    class __declspec(novtable) CullingFilter :
        public InheritMeta<CullingFilter, SceneWalkerFilter> { };
}
