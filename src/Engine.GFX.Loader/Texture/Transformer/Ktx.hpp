#pragma once

#include <Engine.Resource/Source/Source.hpp>

#include "../TextureTransformer.hpp"

namespace ember::engine::gfx {
    class Device;
}

namespace ember::engine::gfx::loader::transformer {
    void convertKtx(
        const non_owning_rptr<const assets::Texture> asset_,
        cref<smr<resource::Source>> src_,
        const non_owning_rptr<VirtualTextureView> dst_
    );

    extern void convertKtx10Gli(
        const non_owning_rptr<const engine::assets::Texture> asset_,
        cref<smr<resource::Source>> src_,
        const non_owning_rptr<VirtualTextureView> dst_,
        cref<sptr<Device>> device_
    );

    extern void convertKtx20(
        const non_owning_rptr<const engine::assets::Texture> asset_,
        cref<smr<resource::Source>> src_,
        const non_owning_rptr<VirtualTextureView> dst_,
        cref<sptr<Device>> device_
    );

    extern void convertKtx20Partial(
        const non_owning_rptr<const engine::assets::Texture> asset_,
        cref<smr<resource::Source>> src_,
        const non_owning_rptr<VirtualTextureView> dst_,
        cref<sptr<Device>> device_,
        const TextureStreamOptions options_
    );

    extern void unloadPartialTmp(
        const non_owning_rptr<const engine::assets::Texture> asset_,
        cref<smr<resource::Source>> src_,
        const non_owning_rptr<VirtualTextureView> dst_,
        cref<sptr<Device>> device_,
        const TextureStreamOptions options_
    );
}
