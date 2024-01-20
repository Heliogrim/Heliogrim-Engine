#pragma once
#include <Engine.Common/Memory/MemoryPointer.hpp>
#include <Engine.GFX.Render.Subpass/Light/LightCaptureInterface.hpp>
#include <Engine.Render.Scene/RenderSceneSystemModel.hpp>

namespace hg::engine::gfx::scene {
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
