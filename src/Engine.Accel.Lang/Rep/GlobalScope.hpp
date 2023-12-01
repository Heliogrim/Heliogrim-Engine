#pragma once
#include "Scope.hpp"

namespace hg::engine::accel::lang {
    struct GlobalScope :
        public Scope {
    public:
        using this_type = GlobalScope;

    public:
        constexpr GlobalScope() noexcept = default;

        explicit GlobalScope(cref<this_type> other_);

        GlobalScope(mref<this_type> other_) noexcept = default;

        ~GlobalScope() = default;

    public:
        Vector<uptr<Variable>> inbound;
        Vector<uptr<Variable>> outbound;
    };
}
