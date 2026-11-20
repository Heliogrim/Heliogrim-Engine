#pragma once

#include "Annotation.hpp"
#include "Type.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::accel::lang {
    struct Variable {
    public:
        using this_type = Variable;

    public:
        constexpr Variable() noexcept = default;

        explicit Variable(cref<this_type> other_);

        Variable(mref<this_type> other_) noexcept = default;

        ~Variable() = default;

    public:
        Type type;
        uptr<Annotation> annotation;

    public:
        u32 countTag;
    };
}
