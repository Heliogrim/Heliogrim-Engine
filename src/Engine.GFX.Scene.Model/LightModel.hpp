#pragma once
#include <Engine.Common/Memory/MemoryPointer.hpp>
#include <Engine.GFX.Render.Subpass/Light/LightCaptureInterface.hpp>

#include "SceneModel.hpp"

namespace hg::engine::gfx::scene {
    class LightModel :
        public InheritMeta<LightModel, SceneModel> {
    public:
        using this_type = LightModel;
        using underlying_type = SceneModel;

    protected:
        LightModel(const ptr<SceneComponent> owner_);

    public:
        ~LightModel() override = default;

    public:
        virtual void capture(nmpt<render::LightCaptureInterface> lci_) const noexcept = 0;
    };
}
