#pragma once
#include <Engine.Common/__macro.hpp>

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

//#include <Engine.Graphics/Scene/LightModel.hpp>
namespace hg::engine::graphics {
    class LightModel;
}

#include "LightCaptureInterface.hpp"
#include "../SubPass.hpp"

namespace hg::engine::render {
    class macro_novtable LightSubPassBase :
        public SubPass { };

    class macro_novtable LightSubPass :
        public LightSubPassBase {
    public:
        using this_type = LightSubPass;

        using model_type = graphics::LightModel;
        using capture_type = LightCaptureInterface;

    public:
        [[nodiscard]] ptr<capture_type> createCaptureObject() {
            return nullptr;
        }
    };
}
