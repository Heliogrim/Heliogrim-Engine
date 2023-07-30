#pragma once

#include <Engine.Common/String.hpp>

namespace hg::engine::gfx::acc {
    enum class SymbolScopeType {
        eGlobal,
        eMaterial,
        eLocal,
        eUserDefined
    };

    struct SymbolScope {
        SymbolScopeType type;
        string layer;
    };
}
