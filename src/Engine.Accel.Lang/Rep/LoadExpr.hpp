#pragma once
#include <Engine.Common/Memory/MemoryPointer.hpp>

#include "Expression.hpp"
#include "Variable.hpp"

namespace hg::engine::accel::lang {
    struct LoadExpr :
        public Expression {
    public:
        using this_type = LoadExpr;

    public:
        constexpr LoadExpr() noexcept = default;

        explicit LoadExpr(cref<this_type> other_);

        LoadExpr(mref<this_type> other_) noexcept = default;

        ~LoadExpr() = default;

    public:
        nmpt<Variable> src;
        nmpt<Variable> dst;
        uptr<Variable> ret;
    };

    struct SubLoadExpr :
        LoadExpr { };
}
