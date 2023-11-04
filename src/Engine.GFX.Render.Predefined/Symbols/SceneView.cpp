#include "SceneView.hpp"
#include "../Descriptions/SceneView.hpp"

#include <Engine.Common/Make.hpp>

using namespace hg::engine::gfx::render;
using namespace hg;

smr<const engine::gfx::acc::Symbol> engine::gfx::render::makeSceneViewSymbol() {

    static auto obj = make_smr<acc::Symbol>(
        acc::SymbolScope {
            .type = acc::SymbolScopeType::eGlobal,
            .layer = ""
        },
        "scene::view",
        makeSceneViewDescription(),
        acc::SymbolFlagBits::eUndefined,
        acc::TransferToken {}
    );
    return obj;
}
