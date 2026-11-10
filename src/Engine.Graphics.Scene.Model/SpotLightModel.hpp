#pragma once

#include "LightModel.hpp"

namespace hg::engine::gfx::scene {
    class SpotLightModel final :
        public InheritMeta<SpotLightModel, LightModel> {
    public:
        using this_type = SpotLightModel;

    public:
        SpotLightModel(const ptr<SceneComponent> owner_);

        SpotLightModel(mref<this_type>) noexcept = default;

        ~SpotLightModel() override;

    public:
        ref<this_type> operator=(mref<this_type>) noexcept = default;

    private:
    public:
        u32 _sceneLightIndex;

    public:
        void create(const ptr<render::RenderSceneSystem> system_) override;

        void update(const ptr<render::RenderSceneSystem> system_) override;

        void destroy(const ptr<render::RenderSceneSystem> system_) override;

    public:
        void capture(nmpt<render::LightCaptureInterface> lci_) const noexcept override;
    };
}
