#pragma once
#include <Engine.Accel.Lang/Rep/Symbol.hpp>
#include <Engine.Common/Collection/DenseMap.hpp>

#include "ResourceView.hpp"

namespace hg::engine::render {
    class ResourceTable {
    public:
        using this_type = ResourceTable;

        using SymbolId = ::hg::engine::accel::lang::SymbolId;

    public:
        DenseMap<SymbolId, ResourceView> table;

    public:
        void bind(cref<SymbolId> symbolId_, mref<ResourceView> view_);
    };
}
