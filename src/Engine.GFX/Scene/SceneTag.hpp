#pragma once

#include <Engine.Scene/SceneTag.hpp>

namespace ember::engine::gfx {

    struct GfxSceneTag :
        public scene::SceneTag {

        static constexpr type_id type_id = "GfxSceneGraphTag"_typeId;

        constexpr GfxSceneTag() noexcept :
            scene::SceneTag(type_id) {}
    };
}
