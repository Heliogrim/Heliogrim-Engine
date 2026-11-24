#pragma once

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Graphics.Material;
import Engine.Resource;
#endif

namespace hg::engine::graphics {
    struct MaterialPrototypeWrapper {
        smr<material::MaterialPrototype> instance;
    };

    typedef resource::Resource<MaterialPrototypeWrapper> MaterialPrototypeResource;
}
