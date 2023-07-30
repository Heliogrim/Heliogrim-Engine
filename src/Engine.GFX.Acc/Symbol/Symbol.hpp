#pragma once

#include <Engine.Common/Concurrent/SharedMemoryReference.hpp>
#include <Engine.GFX.RenderGraph/Relation/Description.hpp>

#include "Flags.hpp"
#include "Scope.hpp"
#include "../Stage/TransferToken.hpp"

namespace hg::engine::gfx::acc {
    class Symbol final {
    public:
        using this_type = Symbol;

    public:
        SymbolScope scope;
        string name;

    public:
        smr<render::graph::Description> description;
        SymbolFlags flags;

    public:
        TransferToken token;
    };
}
