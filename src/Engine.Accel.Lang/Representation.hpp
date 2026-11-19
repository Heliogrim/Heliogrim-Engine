#pragma once

#include <Engine.Common/__macro.hpp>

#include "Rep/GlobalScope.hpp"
#include "Rep/Symbol.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::accel::lang {
    struct Representation {
    public:
        using this_type = Representation;

    public:
        Representation() noexcept = default;

        explicit Representation(cref<this_type> other_);

        Representation(mref<this_type> other_) noexcept = default;

        ~Representation() = default;

    public:
        GlobalScope globalScope;
        nmpt<Function> mainEntry;
        DenseSet<uptr<Symbol>> symbolTable;
    };
}
