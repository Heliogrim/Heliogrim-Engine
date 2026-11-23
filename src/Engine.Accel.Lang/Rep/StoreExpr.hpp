#pragma once

#include "Expression.hpp"
#include "Variable.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
import Engine.Graphics;
#endif

namespace hg::engine::accel::lang {
    struct StoreExpr :
        public Expression {
    public:
        using this_type = StoreExpr;

    public:
        constexpr StoreExpr() noexcept = default;

        explicit StoreExpr(cref<this_type> other_);

        StoreExpr(mref<this_type> other_) noexcept = default;

        ~StoreExpr() = default;

    public:
        nmpt<Variable> src;
        nmpt<Variable> dst;
        uptr<Variable> ret;
    };

    struct SubStoreExpr :
        StoreExpr { };
}
