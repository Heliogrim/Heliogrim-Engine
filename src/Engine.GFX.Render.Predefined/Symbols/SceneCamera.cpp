#include "SceneCamera.hpp"
#include "../Descriptions/SceneCameraBuffer.hpp"

#include <Engine.Common/Make.hpp>

using namespace hg::engine::gfx::render;
using namespace hg;

smr<const engine::gfx::acc::Symbol> engine::gfx::render::makeSceneCameraSymbol() {

    static auto obj = make_smr<acc::Symbol>(
        acc::SymbolScope {
            .type = acc::SymbolScopeType::eGlobal,
            .layer = ""
        },
        "scene::camera",
        makeSceneCameraBufferDescription(),
        acc::SymbolFlagBits::eUndefined,
        acc::TransferToken::from("camera")
    );
    return obj;
}
