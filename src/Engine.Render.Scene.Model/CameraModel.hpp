#pragma once
#include <Engine.Common/Concurrent/SharedMemoryReference.hpp>
#include <Engine.Reflect/Inherit/InheritMeta.hpp>
#include <Engine.Render.Scene/RenderSceneSystemModel.hpp>

namespace hg::engine::gfx::scene {
    class SceneView;
}

namespace hg::engine::render {
    class CameraModel final :
        public InheritMeta<CameraModel, RenderSceneSystemModel> {
    public:
        using this_type = CameraModel;

    public:
        CameraModel(const ptr<SceneComponent> owner_);

        CameraModel(mref<this_type>) noexcept = default;

        ~CameraModel() override final;

    public:
        ref<this_type> operator=(mref<this_type>) noexcept = default;

    public:
        void create(const ptr<RenderSceneSystem> system_) override;

        void update(const ptr<RenderSceneSystem> system_) override;

        void destroy(const ptr<RenderSceneSystem> system_) override;

    private:
        smr<gfx::scene::SceneView> _sceneView;

    public:
        [[nodiscard]] smr<const gfx::scene::SceneView> getSceneView() const noexcept;
    };
}
