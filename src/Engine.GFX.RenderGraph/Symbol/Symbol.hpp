#pragma once

#include <Engine.Common/Flag.hpp>
#include <Engine.Common/Concurrent/SharedMemoryReference.hpp>
#include <Engine.GFX.RenderGraph/Relation/Description.hpp>

#include <Engine.Common/String.hpp>

namespace hg::engine::render::graph {
    enum class SymbolScopeType : u8 {
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

    /**/

    enum class SymbolFlagBits : u8 {
        eUndefined = 0b0000,
        eTransient = 0b0001
    };

    typedef Flag<SymbolFlagBits> SymbolFlags;

    /**/

    class Symbol final {
    public:
        using this_type = Symbol;

    public:
        SymbolFlags flags;
        SymbolScope scope;

    public:
        string name;
        smr<const Description> description;
    };
}
