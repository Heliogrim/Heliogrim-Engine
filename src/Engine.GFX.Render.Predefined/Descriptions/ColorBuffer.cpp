#include "ColorBuffer.hpp"

#include <Engine.Common/Make.hpp>
#include <Engine.GFX.RenderGraph/Relation/TextureDescription.hpp>
#include <Engine.Pedantic/Clone/Clone.hpp>

using namespace hg::engine::render;
using namespace hg;

smr<const graph::Description> engine::render::makeColorBufferDescription() {
    static auto obj = make_smr<graph::TextureDescription>(
        graph::DescriptionValue { graph::DescriptionValueMatchingMode::eCovariant, gfx::TextureType::e2d },
        //graph::DescriptionValue { graph::DescriptionValueMatchingMode::eInvariant, TextureFormat::eR8G8B8A8Unorm },
        graph::DescriptionValue { graph::DescriptionValueMatchingMode::eInvariant, gfx::TextureFormat::eB8G8R8A8Unorm },
        graph::DescriptionValue { graph::DescriptionValueMatchingMode::eInvariant, 1u },
        graph::DescriptionValue { graph::DescriptionValueMatchingMode::eIgnored, graph::ActiveMipBitMask {} }
    );
    return clone(obj).into<const graph::Description>();
}
