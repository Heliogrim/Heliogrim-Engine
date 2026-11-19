#pragma once

#include <Engine.Common/__macro.hpp>

#include "Expression.hpp"
#include "Function.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::accel::lang {
    struct CallExpr :
        public Expression {
    public:
        using this_type = CallExpr;

    public:
        constexpr CallExpr() noexcept = default;

        explicit CallExpr(cref<this_type> other_);

        CallExpr(mref<this_type> other_) noexcept = default;

        ~CallExpr() = default;

    public:
        nmpt<Function> function;
        Vector<nmpt<Variable>> args;
        uptr<Variable> ret;
    };
}
