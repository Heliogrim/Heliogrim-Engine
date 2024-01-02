#pragma once

#include "LightModel.hpp"

namespace hg::engine::gfx::scene {
    class DirectionalLightModel final :
        public InheritMeta<DirectionalLightModel, LightModel> {
    public:
        using this_type = DirectionalLightModel;

    public:
        DirectionalLightModel(const ptr<SceneComponent> owner_);

        ~DirectionalLightModel() override;

    private:
    public:
        u32 _sceneLightIndex;

    public:
        [[nodiscard]] math::fvec3 getLightDirection() const noexcept;

    public:
        void create(const ptr<engine::scene::Scene> scene_) override;

        void update(const ptr<engine::scene::Scene> scene_) override;

        void destroy(const ptr<engine::scene::Scene> scene_) override;

    public:
        void capture(nmpt<render::LightCaptureInterface> lci_) const noexcept override;
    };
}
