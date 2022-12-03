#pragma once

#include <Engine.Scene/SceneTag.hpp>

namespace ember::engine::gfx {

    struct GfxSceneTag :
        public ::ember::engine::scene::SceneTag {

        constexpr static inline type_id typeId = "GfxSceneGraphTag"_typeId;

        constexpr GfxSceneTag() noexcept :
            ::ember::engine::scene::SceneTag(typeId) {}
    };
}
