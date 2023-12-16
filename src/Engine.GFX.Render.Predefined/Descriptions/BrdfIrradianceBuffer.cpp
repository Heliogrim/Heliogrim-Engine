#include "BrdfIrradianceBuffer.hpp"

#include <Engine.Common/Make.hpp>
#include <Engine.GFX.RenderGraph/Relation/TextureDescription.hpp>
#include <Engine.Pedantic/Clone/Clone.hpp>

using namespace hg::engine::render;
using namespace hg;

smr<const graph::Description> engine::render::makeBrdfIrradianceBufferDescription() {
    static auto obj = make_smr<graph::TextureDescription>(
        graph::DescriptionValue { graph::DescriptionValueMatchingMode::eCovariant, gfx::TextureType::eCube },
        graph::DescriptionValue { graph::DescriptionValueMatchingMode::eInvariant, gfx::TextureFormat::eR32G32B32A32Sfloat },
        graph::DescriptionValue { graph::DescriptionValueMatchingMode::eInvariant, 1u },
        graph::DescriptionValue { graph::DescriptionValueMatchingMode::eIgnored, graph::ActiveMipBitMask {} }
    );
    return clone(obj).into<const graph::Description>();
}
