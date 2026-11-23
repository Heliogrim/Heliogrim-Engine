#pragma once
#include <Engine.Common/__macro.hpp>
#include <Engine.Graphics.Scene.Model/Capture/LightCaptureInterface.hpp>

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
import Engine.Graphics.RenderGraph;
#endif

namespace hg::engine::graphics {
    class LightModel;
}

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
