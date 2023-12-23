#pragma once
#include <bitset>
#include <Engine.GFX.Scene.Model/SceneModel.hpp>
#include <Engine.Reflect/Inherit/InheritMeta.hpp>

namespace hg::engine::render::graph {
    class __declspec(novtable) SceneHookFilter :
        public InheritBase<SceneHookFilter> {
    public:
        using this_type = SceneHookFilter;

        using checksum_type = u64;
        using scene_model_type = gfx::scene::SceneModel;

    public:
        constexpr SceneHookFilter() noexcept = default;

        constexpr virtual ~SceneHookFilter() noexcept = default;

    public:
        [[nodiscard]] virtual checksum_type checksum() const noexcept = 0;

    public:
        [[nodiscard]] virtual bool operator()(const ptr<scene_model_type> model_) const noexcept = 0;
    };
}
