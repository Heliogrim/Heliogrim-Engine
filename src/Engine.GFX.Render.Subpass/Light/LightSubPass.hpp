#pragma once
#include <Engine.Common/Wrapper.hpp>

//#include <Engine.GFX/Scene/LightModel.hpp>
namespace hg::engine::gfx {
    class LightModel;
}

#include "../SubPass.hpp"
#include "LightCaptureInterface.hpp"

namespace hg::engine::render {
    class macro_novtable LightSubPassBase :
        public SubPass { };

    class macro_novtable LightSubPass :
        public LightSubPassBase {
    public:
        using this_type = LightSubPass;

        using model_type = gfx::LightModel;
        using capture_type = LightCaptureInterface;

    public:
        [[nodiscard]] ptr<capture_type> createCaptureObject() {
            return nullptr;
        }
    };
}
