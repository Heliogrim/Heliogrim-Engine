#pragma once
#include <bitset>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.GFX.Scene.Model/SceneModel.hpp>
#include <Engine.Reflect/Inherit/InheritMeta.hpp>
#include <Engine.Scene/Node/SceneNode.hpp>

namespace hg::engine::render::graph {
    class __declspec(novtable) SceneWalkerFilter :
        public InheritBase<SceneWalkerFilter> {
    public:
        using this_type = SceneWalkerFilter;

        using checksum_type = u64;

        using scene_model_type = gfx::scene::SceneModel;
        using scene_node_type = ptr<::hg::engine::scene::SceneNode<scene_model_type>>;

    public:
        constexpr SceneWalkerFilter() noexcept = default;

        constexpr virtual ~SceneWalkerFilter() noexcept = default;

    public:
        [[nodiscard]] virtual checksum_type checksum() const noexcept = 0;

    public:
        virtual void update(cref<gfx::scene::SceneView> sceneView_) noexcept = 0;

        [[nodiscard]] virtual bool operator()(const scene_node_type node_) const noexcept = 0;
    };
}
