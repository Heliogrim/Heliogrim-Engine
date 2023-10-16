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

    [[nodiscard]] inline bool operator==(cref<SymbolScope> left_, cref<SymbolScope> right_) noexcept {
        return left_.type == right_.type && left_.layer == right_.layer;
    }
}
