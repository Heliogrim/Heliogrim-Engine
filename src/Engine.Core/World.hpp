#pragma once

#include <Engine.Common/Wrapper.hpp>

namespace hg::engine::scene {
    class Scene;
}

namespace hg::engine::core {
    class World {
    public:
        using this_type = World;

        using Scene = ::hg::engine::scene::Scene;

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
