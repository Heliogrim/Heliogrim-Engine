#pragma once

#include <Engine.Common/Collection/Vector.hpp>
#include <Engine.GFX/Scene/SceneNodeModel.hpp>
#include <Engine.Scene/Node/SceneNode.hpp>

#include "SceneWalkerFilter.hpp"

namespace hg::engine::gfx::render::pipeline {
    class SceneWalker {
    public:
        using scene_model_type = gfx::SceneNodeModel;
        using scene_node_type = ptr<scene::SceneNode<scene_model_type>>;

    private:
        Vector<SceneWalkerFilter> _filter;
        _STD function<void(const ptr<const gfx::SceneNodeModel> model_)> _hook;

    public:
        void hook(mref<decltype(_hook)> hook_);

        template <typename Fn_>
        void hook(Fn_&& fn_) {
            hook(decltype(_hook) { _STD forward<Fn_>(fn_) });
        }

    public:
        [[nodiscard]] bool operator()(const scene_node_type node_);

        [[nodiscard]] bool operator()(u32 batch_, const scene_node_type node_);
    };
}
