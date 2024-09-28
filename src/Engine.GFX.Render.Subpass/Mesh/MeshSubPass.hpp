#pragma once
#include <Engine.Common/Wrapper.hpp>
#include <Engine.GFX.Scene.Model/GeometryModel.hpp>

#include "../SubPass.hpp"
#include "MeshCaptureInterface.hpp"

namespace hg::engine::render {
    class macro_novtable MeshSubPassBase :
        public SubPass { };

    class macro_novtable MeshSubPass :
        public MeshSubPassBase {
    public:
        using this_type = MeshSubPass;

        using model_type = gfx::scene::GeometryModel;
        using capture_type = MeshCaptureInterface;

    public:
        [[nodiscard]] ptr<capture_type> createCaptureObject() {
            return nullptr;
        }
    };
}
