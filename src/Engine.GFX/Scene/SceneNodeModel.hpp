#pragma once

#include <Engine.Scene/SceneNodeModel.hpp>
#include <Engine.Scene/Node/SceneNodePath.hpp>

#include "../Cache/ModelBatch.hpp"

namespace hg::engine::gfx {
    class __declspec(novtable) SceneNodeModel :
        public InheritMeta<SceneNodeModel, ::hg::engine::scene::SceneNodeModel> {
    public:
        SceneNodeModel(const ptr<SceneComponent> owner_);

    private:
        // TODO: Check whether we want this in basic class of scene nodes
        ::hg::engine::scene::SceneNodePath _sceneNodePath;

    public:
        [[nodiscard]] ::hg::engine::scene::SceneNodePath sceneNodePath() const noexcept {
            return _sceneNodePath;
        }

        void setSceneNodePath(const ::hg::engine::scene::SceneNodePath path_) noexcept {
            _sceneNodePath = path_;
        }
    };
}
