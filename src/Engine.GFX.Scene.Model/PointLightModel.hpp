#pragma once

#include "LightModel.hpp"

namespace hg::engine::gfx::scene {
    class PointLightModel final :
        public InheritMeta<PointLightModel, LightModel> {
    public:
        using this_type = PointLightModel;

    public:
        PointLightModel(const ptr<SceneComponent> owner_);

        ~PointLightModel() override;

    private:
    public:
        u32 _sceneLightIndex;

    public:
        void create(const ptr<engine::scene::Scene> scene_) override;

        void update(const ptr<engine::scene::Scene> scene_) override;

        void destroy(const ptr<engine::scene::Scene> scene_) override;

    public:
        void capture(nmpt<render::LightCaptureInterface> lci_) const noexcept override;
    };
}
