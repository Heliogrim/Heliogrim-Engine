#pragma once
#include <Engine.Common/String.hpp>
#include <Engine.Common/Collection/Vector.hpp>

#include "ILDialect.hpp"

namespace hg::engine::gfx::acc::lang {
    class IL {
    public:
        ILDialect _dialect;
        Vector<string> _snippets;
    };
}
