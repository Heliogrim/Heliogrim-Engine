#pragma once
#include <Engine.Accel.Lang/Rep/Symbol.hpp>
#include <Engine.Common/Collection/Vector.hpp>
#include <Engine.Common/Memory/MemoryPointer.hpp>

namespace hg::engine::accel {
    struct BindElement {
        nmpt<const lang::Symbol> symbol;
    };

    struct BindGroup {
        Vector<BindElement> elements;
    };

    struct BindLayout {
        Vector<BindGroup> groups;
    };
}
