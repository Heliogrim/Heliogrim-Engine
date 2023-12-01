#pragma once
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Collection/Vector.hpp>

#include "Function.hpp"
#include "Variable.hpp"

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
