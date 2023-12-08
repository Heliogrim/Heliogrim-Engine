#include "DepthStencilBuffer.hpp"

#include <Engine.Common/Make.hpp>
#include <Engine.GFX.RenderGraph/Relation/TextureDescription.hpp>
#include <Engine.Pedantic/Clone/Clone.hpp>

using namespace hg::engine::render;
using namespace hg;

// Warning;
constexpr static auto global_depth_stencil_format = engine::gfx::TextureFormat::eD24UnormS8Uint;

smr<const graph::Description> engine::render::makeDepthStencilBufferDescription() {
    static auto obj = make_smr<graph::TextureDescription>(
        graph::DescriptionValue { graph::DescriptionValueMatchingMode::eInvariant, gfx::TextureType::e2d },
        graph::DescriptionValue { graph::DescriptionValueMatchingMode::eInvariant, global_depth_stencil_format },
        graph::DescriptionValue { graph::DescriptionValueMatchingMode::eInvariant, 1u },
        graph::DescriptionValue { graph::DescriptionValueMatchingMode::eIgnored, graph::ActiveMipBitMask {} }
    );
    return clone(obj).into<const graph::Description>();
}
