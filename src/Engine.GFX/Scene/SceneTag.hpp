#pragma once

#include <Engine.Scene/SceneTag.hpp>

namespace ember::engine::gfx {

    struct GfxSceneTag :
        public scene::SceneTag {

        static constexpr type_id typeId = "GfxSceneGraphTag"_typeId;

        constexpr GfxSceneTag() noexcept :
            scene::SceneTag(typeId) {}
    };
}
