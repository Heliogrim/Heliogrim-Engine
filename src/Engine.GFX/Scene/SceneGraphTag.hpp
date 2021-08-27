#pragma once

#include <Engine.Scene/Graph/SceneGraphTag.hpp>

namespace ember::engine::gfx {

    struct GfxSceneGraphTag :
        scene::SceneGraphTag<GfxSceneGraphTag> {

        static constexpr type_id type_id = "GfxSceneGraphTag"_typeId;

        constexpr GfxSceneGraphTag() = default;
    };

}
