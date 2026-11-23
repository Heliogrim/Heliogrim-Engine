#pragma once
#include <Engine.Accel.Effect/Stage/StageFlags.hpp>

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Accel.Lang;
import Engine.Common;
#endif

namespace hg::engine::accel {
    enum class BindType {
        eUniformBuffer,
        eStorageBuffer,
        eTexture,
        eTextureSampler,
        eStorageImage
    };

    struct BindElement {
        nmpt<const lang::Symbol> symbol;
        BindType type;
    };

    struct BindGroup {
        Vector<BindElement> elements;
        ptr<void> drvAux;
    };

    struct PushConstant {
        StageFlags stages;
        u32 offset;
        u32 size;
    };

    struct BindLayout {
        Vector<BindGroup> groups;
        Vector<PushConstant> constants;
    };
}
