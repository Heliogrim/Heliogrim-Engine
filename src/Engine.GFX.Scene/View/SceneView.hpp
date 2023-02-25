#pragma once

#include <Engine.Common/Wrapper.hpp>

#include "SceneViewEye.hpp"

namespace ember {
    class Actor;
}

namespace ember::engine::scene {
    class IRenderScene;
}

namespace ember::engine::gfx::scene {
    class SceneView :
        public SceneViewEye {
    public:
        using this_type = SceneView;

        using RenderScene = ::ember::engine::scene::IRenderScene;

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
