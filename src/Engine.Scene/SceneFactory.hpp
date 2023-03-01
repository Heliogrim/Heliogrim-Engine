#pragma once
#include <Engine.Common/Wrapper.hpp>

#include "Scene.hpp"

namespace hg::engine::scene {
    class SceneFactory {
    public:
        using this_type = SceneFactory;

    public:
        SceneFactory() = default;

        ~SceneFactory() = default;

    public:
        [[nodiscard]] static uptr<Scene> createDefaultScene();

        [[nodiscard]] static uptr<Scene> createPreviewScene();
    };
}
