#pragma once
#include <Engine.Graphics.Render.Subpass/Light/LightCaptureInterface.hpp>

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
import Engine.Render.Scene;
#endif

namespace hg::engine::graphics::scene {
    class LightModel :
        public InheritMeta<LightModel, render::RenderSceneSystemModel> {
    public:
        using this_type = LightModel;

    protected:
        LightModel(const ptr<SceneComponent> owner_);

    public:
        LightModel(mref<this_type>) noexcept = default;

        ~LightModel() override = default;

    public:
        ref<this_type> operator=(mref<this_type>) noexcept = default;

    public:
        virtual void capture(nmpt<render::LightCaptureInterface> lci_) const noexcept = 0;
    };
}
