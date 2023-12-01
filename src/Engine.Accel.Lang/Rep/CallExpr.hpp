#pragma once

#include <Engine.Common/Collection/Vector.hpp>
#include <Engine.Common/Memory/MemoryPointer.hpp>

#include "Expression.hpp"
#include "Function.hpp"

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
