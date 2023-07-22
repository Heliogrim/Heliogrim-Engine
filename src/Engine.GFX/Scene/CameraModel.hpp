#pragma once

#include "SceneNodeModel.hpp"

namespace hg::engine::gfx::scene {
    class SceneView;
}

namespace hg::engine::gfx {
    class CameraModel final :
        public InheritMeta<CameraModel, SceneNodeModel> {
    public:
        using this_type = CameraModel;

        inline constexpr static type_id typeId { "CameraModel"_typeId };

    public:
        CameraModel(const non_owning_rptr<SceneComponent> owner_);

        ~CameraModel() override final;

    public:
        void create(const ptr<engine::scene::Scene> scene_) override;

        void update(const ptr<engine::scene::Scene> scene_) override;

        void destroy(const ptr<engine::scene::Scene> scene_) override;

    public:
        [[nodiscard]] Vector<render::RenderDataToken> providedToken() const noexcept override;

        [[nodiscard]] ptr<cache::ModelBatch> batch(const ptr<render::RenderPassState> state_) override;

    private:
        uptr<scene::SceneView> _sceneView;

    public:
        [[nodiscard]] const non_owning_rptr<scene::SceneView> getSceneView() const noexcept;
    };
}
