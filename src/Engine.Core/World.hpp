#pragma once

#include <Engine.Common/Wrapper.hpp>

namespace ember::engine::scene {
    class Scene;
}

namespace ember::engine::core {

    class World {
    public:
        using this_type = World;

        using Scene = ::ember::engine::scene::Scene;

    public:
        World(mref<uptr<Scene>> scene_);

        World(cref<this_type>) = delete;

        World(mref<this_type>) noexcept = delete;

        ~World();

    private:
        uptr<Scene> _scene;

    public:
        [[nodiscard]] const non_owning_rptr<Scene> getScene() const noexcept;
    };

}
