#include "SkyboxTexture.hpp"

#include <Engine.Common/Make.hpp>

#include "../Descriptions/SkyboxColorBuffer.hpp"

using namespace hg::engine::render;
using namespace hg;

smr<const graph::Symbol> engine::render::makeSkyboxTextureSymbol() {
    static auto obj = make_smr<graph::Symbol>(
        graph::SymbolFlagBits::eUndefined,
        graph::SymbolScope {
            .type = graph::SymbolScopeType::eGlobal,
            .layer = ""
        },
        "skybox::color",
        makeSkyboxColorBufferDescription()
    );
    return obj;
}
