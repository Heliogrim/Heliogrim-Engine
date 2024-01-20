#pragma once

#include <Engine.Common/Wrapper.hpp>

namespace hg::engine::scene {
    class SceneBase;
}

namespace hg::engine::core {
    class World {
    public:
        using this_type = World;

    public:
        World(mref<uptr<scene::SceneBase>> scene_);

        World(cref<this_type>) = delete;

        World(mref<this_type>) noexcept = delete;

        ~World();

    private:
        uptr<scene::SceneBase> _scene;

    public:
        [[nodiscard]] const non_owning_rptr<scene::SceneBase> getScene() const noexcept;
    };
}
