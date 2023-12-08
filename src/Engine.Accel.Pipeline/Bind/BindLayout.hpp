#pragma once
#include <Engine.Accel.Lang/Rep/Symbol.hpp>
#include <Engine.Common/Collection/Vector.hpp>
#include <Engine.Common/Memory/MemoryPointer.hpp>

namespace hg::engine::accel {
    enum class BindType {
        eUniformBuffer,
        eStorageBuffer,
        eTexture
    };

    struct BindElement {
        nmpt<const lang::Symbol> symbol;
        BindType type;
    };

    struct BindGroup {
        Vector<BindElement> elements;
        ptr<void> drvAux;
    };

    struct BindLayout {
        Vector<BindGroup> groups;
    };
}
