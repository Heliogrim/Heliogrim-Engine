#pragma once
#include "../SceneWalkerFilter.hpp"

namespace hg::engine::render::graph {
    class __declspec(novtable) CullingFilter :
        public InheritMeta<CullingFilter, SceneWalkerFilter> { };
}
