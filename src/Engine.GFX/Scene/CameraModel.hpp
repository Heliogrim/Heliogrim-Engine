#pragma once

#include <Engine.Common/Concurrent/SharedMemoryReference.hpp>

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

    private:
        smr<scene::SceneView> _sceneView;

    public:
        [[nodiscard]] smr<const scene::SceneView> getSceneView() const noexcept;
    };
}
