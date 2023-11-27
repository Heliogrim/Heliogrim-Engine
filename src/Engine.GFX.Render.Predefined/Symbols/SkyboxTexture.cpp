#include "SkyboxTexture.hpp"

#include <Engine.Common/Make.hpp>

#include "../Descriptions/SkyboxColorBuffer.hpp"

using namespace hg::engine::gfx::render;
using namespace hg;

smr<const engine::gfx::acc::Symbol> engine::gfx::render::makeSkyboxTextureSymbol() {
    static auto obj = make_smr<acc::Symbol>(
        acc::SymbolScope {
            .type = acc::SymbolScopeType::eGlobal,
            .layer = ""
        },
        "skybox::color",
        makeSkyboxColorBufferDescription(),
        acc::SymbolFlagBits::eUndefined,
        acc::TransferToken {}
    );
    return obj;
}
