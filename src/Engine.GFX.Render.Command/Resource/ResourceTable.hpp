#pragma once
#include <Engine.Accel.Lang/Rep/Symbol.hpp>
#include <Engine.Common/Collection/DenseMap.hpp>

#include "Resource.hpp"

namespace hg::engine::render {
    class __declspec(novtable) ResourceTable {
    public:
        using this_type = ResourceTable;

        using SymbolId = ::hg::engine::accel::lang::SymbolId;
        using Resource = ::hg::engine::render::Resource;

    public:
        virtual ~ResourceTable() noexcept = default;

    public:
        virtual void bind(cref<SymbolId> symbolId_, mref<Resource> resource_) = 0;
    };
}
