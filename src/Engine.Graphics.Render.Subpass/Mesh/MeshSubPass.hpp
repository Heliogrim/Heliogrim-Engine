#pragma once
#include <Engine.Common/__macro.hpp>

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
import Engine.Graphics.RenderGraph;
import Engine.Graphics.Scene.Model;
#endif

namespace hg::engine::render {
    class macro_novtable MeshSubPassBase :
        public SubPass { };

    class macro_novtable MeshSubPass :
        public MeshSubPassBase {
    public:
        using this_type = MeshSubPass;

        using model_type = graphics::scene::GeometryModel;
        using capture_type = MeshCaptureInterface;

    public:
        [[nodiscard]] ptr<capture_type> createCaptureObject() {
            return nullptr;
        }
    };
}
