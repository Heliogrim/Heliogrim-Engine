#pragma once

#include <Engine.Common/__macro.hpp>

#include "Annotation.hpp"
#include "Type.hpp"
#include "Variable.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::accel::lang {
    struct Function {
    public:
        using this_type = Function;

    public:
        constexpr Function() noexcept = default;

        explicit Function(cref<this_type> other_);

        Function(mref<this_type> other_) noexcept = default;

        ~Function() = default;

    public:
        Type retType;
        Vector<uptr<Variable>> params;
        uptr<Annotation> annotation;
    };
}
