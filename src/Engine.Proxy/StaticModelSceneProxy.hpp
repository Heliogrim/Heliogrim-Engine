#pragma once
#include <Engine.Common/Wrapper.hpp>

#include "SceneProxy.hpp"

/**
 * Forward Declaration
 */
namespace ember::engine::gfx {
    class SceneElement;
}

namespace ember::engine::res {
    class Resource;
}

namespace ember::engine::proxy {
    class StaticModelSceneProxy :
        public SceneProxy {

    public:
    private:
        sptr<res::Resource> _meshResource;

    private:
        sptr<res::Resource> _materialResource;
    };
}
