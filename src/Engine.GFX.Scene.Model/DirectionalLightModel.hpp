#pragma once

#include <Engine.Common/Math/Vector.hpp>

#include "LightModel.hpp"

namespace hg::engine::gfx::scene {
    class DirectionalLightModel final :
        public InheritMeta<DirectionalLightModel, LightModel> {
    public:
        using this_type = DirectionalLightModel;

    public:
        DirectionalLightModel(const ptr<SceneComponent> owner_);

        DirectionalLightModel(mref<this_type>) noexcept = default;

        ~DirectionalLightModel() override;

    public:
        ref<this_type> operator=(mref<this_type>) noexcept = default;

    private:
    public:
        u32 _sceneLightIndex;
        u32 _sceneShadowIndex;

    public:
        [[nodiscard]] math::fvec3 getLightDirection() const noexcept;

    public:
        void create(const ptr<render::RenderSceneSystem> system_) override;

        void update(const ptr<render::RenderSceneSystem> system_) override;

        void destroy(const ptr<render::RenderSceneSystem> system_) override;

    public:
        void capture(nmpt<render::LightCaptureInterface> lci_) const noexcept override;
    };
}
