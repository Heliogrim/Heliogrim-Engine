#pragma once

#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Memory/MemoryPointer.hpp>

#include "SceneViewEye.hpp"

namespace hg {
    class Actor;
}

namespace hg::engine::render {
    class RenderSceneSystem;
}

namespace hg::engine::gfx::scene {
    class SceneView :
        public SceneViewEye {
    public:
        using this_type = SceneView;

    public:
        SceneView(nmpt<Actor> actor_, nmpt<render::RenderSceneSystem> system_);

        ~SceneView();

    private:
        nmpt<Actor> _actor;
        nmpt<render::RenderSceneSystem> _renderSceneSystem;

    public:
        [[nodiscard]] nmpt<Actor> getViewActor() const noexcept;

        [[nodiscard]] nmpt<render::RenderSceneSystem> getRenderSceneSystem() const noexcept;

    public:
        [[nodiscard]] cref<SceneViewEye> getSceneViewEye() const noexcept;
    };
}
