#pragma once

#include <Engine.Scene/SceneNodeModel.hpp>
#include <Engine.Scene/Node/SceneNodePath.hpp>

namespace hg::engine::gfx::scene {
    class __declspec(novtable) SceneModel :
        public InheritMeta<SceneModel, ::hg::engine::scene::SceneNodeModel> {
    public:
        SceneModel(const ptr<SceneComponent> owner_);

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
