#include "BrdfPrefilterBuffer.hpp"

#include <Engine.Common/Make.hpp>
#include <Engine.Graphics.RenderGraph/Relation/TextureDescription.hpp>
#include <Engine.Pedantic/Clone/Clone.hpp>

using namespace hg::engine::render;
using namespace hg;

smr<const graph::Description> engine::render::makeBrdfPrefilterBufferDescription() {
    static auto obj = make_smr<graph::TextureDescription>(
        graph::DescriptionValue { graph::DescriptionValueMatchingMode::eCovariant, graphics::TextureType::eCube },
        graph::DescriptionValue {
            graph::DescriptionValueMatchingMode::eInvariant, graphics::TextureFormat::eR16G16B16A16Sfloat
        },
        graph::DescriptionValue { graph::DescriptionValueMatchingMode::eInvariant, 1u },
        graph::DescriptionValue { graph::DescriptionValueMatchingMode::eIgnored, graph::ActiveMipBitMask {} }
    );
    return clone(obj).into<const graph::Description>();
}
