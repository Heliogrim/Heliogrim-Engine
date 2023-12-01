#pragma once
#include <Engine.Common/Types.hpp>
#include <Engine.Common/Wrapper.hpp>

#include "Annotation.hpp"
#include "Type.hpp"

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
