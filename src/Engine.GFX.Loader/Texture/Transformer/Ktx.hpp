#pragma once

#include <Engine.Resource/Source/Source.hpp>

#include "../TextureTransformer.hpp"

namespace ember::engine::gfx::loader::transformer {
    void convertKtx(
        const non_owning_rptr<const assets::Texture> asset_,
        cref<smr<resource::Source>> src_,
        const non_owning_rptr<VirtualTextureView> dst_
    );
}
