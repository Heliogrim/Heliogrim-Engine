#pragma once

#include <Engine.Common/Wrapper.hpp>

#include "SceneViewEye.hpp"

namespace hg {
    class Actor;
}

namespace hg::engine::scene {
    class IRenderScene;
}

namespace hg::engine::gfx::scene {
    class SceneView :
        public SceneViewEye {
    public:
        using this_type = SceneView;

        using RenderScene = ::hg::engine::scene::IRenderScene;

    public:
        SceneView(const non_owning_rptr<Actor> actor_, const non_owning_rptr<RenderScene> scene_);

        ~SceneView();

    private:
        non_owning_rptr<Actor> _actor;

    public:
        [[nodiscard]] non_owning_rptr<Actor> getViewActor() const noexcept;

    private:
        non_owning_rptr<RenderScene> _scene;

    public:
        [[nodiscard]] non_owning_rptr<RenderScene> getScene() const noexcept;

    public:
        [[nodiscard]] operator ref<SceneViewEye>() noexcept;

        [[nodiscard]] operator cref<SceneViewEye>() const noexcept;
    };
}
