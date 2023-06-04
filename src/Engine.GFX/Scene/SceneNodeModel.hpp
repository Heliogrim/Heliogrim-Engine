#pragma once

#include <Engine.Scene/SceneNodeModel.hpp>
#include <Engine.GFX.Renderer/RenderPassState.hpp>
#include <Engine.GFX.Renderer/__fwd.hpp>

#include "../Cache/ModelBatch.hpp"

#include "Engine.Scene/Node/SceneNodePath.hpp"

namespace hg::engine::gfx {
    class __declspec(novtable) SceneNodeModel :
        public ::hg::engine::scene::SceneNodeModel {
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
