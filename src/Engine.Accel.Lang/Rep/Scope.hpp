#pragma once

#include <Engine.Common/__macro.hpp>

#include "Function.hpp"
#include "Variable.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::accel::lang {
    struct Scope {
    public:
        using this_type = Scope;

    public:
        constexpr Scope() noexcept = default;

        explicit Scope(cref<this_type> other_);

        Scope(mref<this_type> other_) noexcept = default;

        ~Scope() = default;

    public:
        Vector<uptr<Function>> functions;
        Vector<uptr<Variable>> variables;
    };
}
