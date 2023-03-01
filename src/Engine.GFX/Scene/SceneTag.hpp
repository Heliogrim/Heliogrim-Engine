#pragma once

#include <Engine.Scene/SceneTag.hpp>

namespace hg::engine::gfx {
    struct GfxSceneTag :
        public ::hg::engine::scene::SceneTag {
        constexpr static inline type_id typeId = "GfxSceneGraphTag"_typeId;

        constexpr GfxSceneTag() noexcept :
            ::hg::engine::scene::SceneTag(typeId) {}
    };
}
