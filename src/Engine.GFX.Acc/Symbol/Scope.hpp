#pragma once

#include <Engine.Common/String.hpp>

namespace hg::engine::gfx::acc {
    enum class SymbolScopeType {
        // ...
        eGlobal,

        // ...
        eMaterial,

        // Should scope resources which may be expose under circumstances,
        //  but are treated as local to the exposing instance.
        eLocal,

        // Should scope resources which are spread out, but should only be touched
        //  by the emitted procedures related to a unique effect.
        eEffectInternal,

        /**/
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
