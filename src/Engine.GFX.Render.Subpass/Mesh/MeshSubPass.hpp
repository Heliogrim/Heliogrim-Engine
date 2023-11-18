#pragma once
#include <Engine.Common/Wrapper.hpp>
#include <Engine.GFX/Scene/GeometryModel.hpp>

#include "../SubPass.hpp"
#include "MeshCaptureInterface.hpp"

namespace hg::engine::gfx::render {
    class __declspec(novtable) MeshSubPassBase :
        public SubPass { };

    class __declspec(novtable) MeshSubPass :
        public MeshSubPassBase {
    public:
        using this_type = MeshSubPass;

        using model_type = GeometryModel;
        using capture_type = MeshCaptureInterface;

    public:
        [[nodiscard]] ptr<capture_type> createCaptureObject() {
            return nullptr;
        }
    };
}
