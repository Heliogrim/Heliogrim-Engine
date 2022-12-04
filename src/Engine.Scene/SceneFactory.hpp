#pragma once
#include <Engine.Common/Wrapper.hpp>

#include "Scene.hpp"

namespace ember::engine::scene {

    class SceneFactory {
    public:
        using this_type = SceneFactory;

    public:
        SceneFactory() = default;

        ~SceneFactory() = default;

    public:
        [[nodiscard]] static sptr<Scene> createDefaultScene();

        [[nodiscard]] static sptr<Scene> createPreviewScene();
    };
}
