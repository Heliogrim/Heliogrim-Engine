#include "SkyboxColorBuffer.hpp"

#include <Engine.Common/Make.hpp>
#include <Engine.GFX.RenderGraph/Relation/TextureDescription.hpp>
#include <Engine.Pedantic/Clone/Clone.hpp>

using namespace hg::engine::gfx::render;
using namespace hg;

smr<const graph::Description> engine::gfx::render::makeSkyboxColorBufferDescription() {
    static auto obj = make_smr<graph::TextureDescription>(
        graph::DescriptionValue { graph::DescriptionValueMatchingMode::eCovariant, TextureType::eCube },
        graph::DescriptionValue { graph::DescriptionValueMatchingMode::eInvariant, TextureFormat::eR16G16B16A16Sfloat },
        graph::DescriptionValue { graph::DescriptionValueMatchingMode::eInvariant, 1u },
        graph::DescriptionValue { graph::DescriptionValueMatchingMode::eIgnored, graph::ActiveMipBitMask {} }
    );
    return clone(obj).into<const graph::Description>();
}
