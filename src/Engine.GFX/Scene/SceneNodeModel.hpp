#pragma once

#include <Engine.Scene/SceneNodeModel.hpp>

#include "../Cache/ModelBatch.hpp"
#include "../Renderer/RenderPassState.hpp"
#include "../Renderer/__fwd.hpp"

#include "Engine.Scene/Node/SceneNodePath.hpp"

namespace ember::engine::gfx {

    class __declspec(novtable) SceneNodeModel :
        public ::ember::engine::scene::SceneNodeModel {
    public:
        SceneNodeModel(const ptr<SceneComponent> owner_);

    public:
        [[nodiscard]] virtual Vector<render::RenderDataToken> providedToken() const noexcept = 0;

    public:
        /**
         * Request a derived ModelBatch from the current SceneNodeModel use for the current RenderPass (Context).
         *
         * @details This might try to fetch a previous ModelBatch mapped by this component at the RenderPassState's LocalResourceCache.
         *  If the result of the fetch is a CacheResult::eMiss, this call might allocate the requested resources and store a copy
         *  of the newly created ModelBatch at the LocalResourceCache for further access/reuse.
         *
         * @param state_ The pointer to the RenderPassState of the current RenderPass which requests data to render
         *
         * @returns A pointer to a specialized ModelBatch containing the contract resources to render this model representation.
         */
        [[nodiscard]] virtual ptr<cache::ModelBatch> batch(const ptr<render::RenderPassState> state_) = 0;

    private:
        // TODO: Check whether we want this in basic class of scene nodes
        ::ember::engine::scene::SceneNodePath _sceneNodePath;

    public:
        [[nodiscard]] ::ember::engine::scene::SceneNodePath sceneNodePath() const noexcept {
            return _sceneNodePath;
        }

        void setSceneNodePath(const ::ember::engine::scene::SceneNodePath path_) noexcept {
            _sceneNodePath = path_;
        }
    };
}
