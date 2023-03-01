#include "RevVirtualMarkerTexture.hpp"

#include <Engine.GFX/Buffer/Buffer.hpp>
#include <Engine.GFX/Command/CommandBuffer.hpp>
#include <Engine.GFX/Command/CommandQueue.hpp>
#include <Engine.GFX/Texture/TextureFactory.hpp>

using namespace hg::engine::gfx::glow;
using namespace hg::engine::gfx;
using namespace hg;

constexpr u16 internal_mip_mask { 0b1111'000000'000000ui16 };
constexpr u16 internal_mip_shift { 12ui16 };

constexpr u16 internal_uvw_bit_per_dim { 6ui16 };
constexpr u16 internal_uvw_dim_mask { 0b0000'000000'111111ui16 };
constexpr u16 internal_uvw_mask { 0b0000'111111'111111ui16 };
constexpr u16 internal_uvw_shift { 0ui16 };

RevVirtualMarkerTexture::~RevVirtualMarkerTexture() {
    if (_texture) {
        destroy();
    }
}

void RevVirtualMarkerTexture::generateMarkers() {

    Buffer stage {};
    const auto device { TextureFactory::get()->device() };

    /**
     * Setup vulkan stage buffer
     */
    const s64 tileDataSize {
        _tileExtent.x * _tileExtent.y * _tileExtent.z *
        formatDataSize(TextureFormat::eR16Uint)
    };

    const s64 stageSize {
        _tileExtent.x * _tileExtent.y * _tileExtent.z *
        formatDataSize(TextureFormat::eR8G8B8A8Unorm)
    };

    vk::BufferCreateInfo bci {
        vk::BufferCreateFlags(),
        static_cast<u64>(stageSize),
        vk::BufferUsageFlagBits::eTransferSrc,
        vk::SharingMode::eExclusive,
        0,
        nullptr
    };

    stage.buffer = device->vkDevice().createBuffer(bci);
    stage.device = device->vkDevice();

    const auto allocResult {
        memory::allocate(
            device->allocator(),
            device,
            stage.buffer,
            MemoryProperties { MemoryProperty::eHostVisible },
            stage.memory
        )
    };
    assert(stage.buffer);
    assert(stage.memory);

    stage.size = stage.memory->size;
    stage.usageFlags = vk::BufferUsageFlagBits::eTransferSrc;

    /**
     *
     */
    stage.bind();
    stage.mapAligned();
    assert(stage.memory->mapping);

    /**
     * Transform and data transfer
     */
    auto pool = device->transferQueue()->pool();
    pool->lck().acquire();
    CommandBuffer cmd = pool->make();

    /**/
    vk::ImageMemoryBarrier preBarrier {};
    vk::ImageMemoryBarrier postBarrier {};
    /**/

    /**
     * Prepare Layout
     */
    cmd.begin();
    preBarrier = {
        vk::AccessFlags(),
        vk::AccessFlags(),
        vk::ImageLayout::eUndefined,
        vk::ImageLayout::eTransferDstOptimal,
        VK_QUEUE_FAMILY_IGNORED,
        VK_QUEUE_FAMILY_IGNORED,
        _texture.buffer().image(),
        {
            vk::ImageAspectFlagBits::eColor,
            0,
            _supportedMips.max,
            0ui32,
            1ui32
        }
    };

    cmd.vkCommandBuffer().pipelineBarrier(
        vk::PipelineStageFlagBits::eTransfer,
        vk::PipelineStageFlagBits::eTransfer,
        vk::DependencyFlags(),
        0,
        nullptr,
        0,
        nullptr,
        1,
        &preBarrier
    );

    cmd.end();
    cmd.submitWait();

    /**
     * Generate and Copy data
     */

    for (u32 mip { _supportedMips.min }; mip < _supportedMips.max; ++mip) {

        //
        auto extent { _extent >> mip };
        extent.x = extent.x > 0ui32 ? extent.x : 1ui32;
        extent.y = extent.y > 0ui32 ? extent.y : 1ui32;
        extent.z = extent.z > 0ui32 ? extent.z : 1ui32;

        const math::uivec3 tile {
            (extent.x / _tileExtent.x) + ((extent.x % _tileExtent.x) ? 1ui32 : 0ui32),
            (extent.y / _tileExtent.y) + ((extent.y % _tileExtent.y) ? 1ui32 : 0ui32),
            (extent.z / _tileExtent.z) + ((extent.z % _tileExtent.z) ? 1ui32 : 0ui32)
        };

        //
        for (u32 tileZ { 0ui32 }; tileZ < tile.z; ++tileZ) {
            for (u32 tileY { 0ui32 }; tileY < tile.y; ++tileY) {
                for (u32 tileX { 0ui32 }; tileX < tile.x; ++tileX) {

                    cmd.reset();
                    cmd.begin();

                    const auto encoded {
                        tileIndex({ tileX * _tileExtent.x, tileY * _tileExtent.y, tileZ * _tileExtent.z }, mip)
                    };

                    for (u32 i { 0ui32 }; i < tileDataSize / sizeof(u16); ++i) {
                        static_cast<ptr<u16>>(stage.memory->mapping)[i] = static_cast<u16>(encoded);
                    }

                    stage.flushAligned(tileDataSize);

                    const vk::BufferImageCopy region {
                        0ui32,
                        0ui32,
                        0ui32,
                        vk::ImageSubresourceLayers {
                            vk::ImageAspectFlagBits::eColor,
                            mip,
                            0ui32,
                            1ui32
                        },
                        vk::Offset3D {
                            static_cast<s32>(tileX * _tileExtent.x),
                            static_cast<s32>(tileY * _tileExtent.y),
                            static_cast<s32>(tileZ * _tileExtent.z)
                        },
                        vk::Extent3D {
                            MIN(_tileExtent.x, extent.x),
                            MIN(_tileExtent.y, extent.y),
                            MIN(_tileExtent.z, extent.z)
                        }
                    };

                    cmd.vkCommandBuffer().copyBufferToImage(
                        stage.buffer,
                        _texture.buffer().image(),
                        vk::ImageLayout::eTransferDstOptimal,
                        1ui32,
                        &region
                    );

                    cmd.end();
                    cmd.submitWait();
                }
            }
        }

    }

    /**
     * Restore Layout
     */
    cmd.reset();
    cmd.begin();
    postBarrier = {
        vk::AccessFlags(),
        vk::AccessFlags(),
        vk::ImageLayout::eTransferDstOptimal,
        vk::ImageLayout::eShaderReadOnlyOptimal,
        VK_QUEUE_FAMILY_IGNORED,
        VK_QUEUE_FAMILY_IGNORED,
        _texture.buffer().image(),
        {
            vk::ImageAspectFlagBits::eColor,
            0,
            _supportedMips.max,
            0ui32,
            1ui32
        }
    };

    cmd.vkCommandBuffer().pipelineBarrier(
        vk::PipelineStageFlagBits::eTransfer,
        vk::PipelineStageFlagBits::eTopOfPipe,
        vk::DependencyFlags(),
        0,
        nullptr,
        0,
        nullptr,
        1,
        &postBarrier
    );

    cmd.end();
    cmd.submitWait();
    cmd.release();

    pool->lck().release();

    /**
     *
     */
    stage.unmap();
    stage.destroy();

    _texture.buffer()._vkAspect = vk::ImageAspectFlagBits::eColor;
    _texture.buffer()._vkLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
}

void RevVirtualMarkerTexture::setup() {

    // TODO: Query device limits and sparse sizes
    _tileExtent = { 128ui32, 128ui32, 1ui32 };
    _extent = { 8192ui32, 8192ui32, 1ui32 };

    //
    const auto mipLimit { static_cast<u32>(_STD log2(MAX(MAX(_extent.x, _extent.y), _extent.z)) + 1.F) };

    _supportedMips = { 0ui32, mipLimit };

    _tilesPerLayer.clear();
    _tilesPerLayer.reserve(mipLimit);

    for (u32 mip { 0ui32 }; mip < mipLimit; ++mip) {

        const auto extent { _extent >> mip };
        const auto tilesPerRow { (extent.x / _tileExtent.x) + ((extent.x % _tileExtent.x) ? 1ui32 : 0ui32) };
        const auto tilesPerCol { (extent.y / _tileExtent.y) + ((extent.y % _tileExtent.y) ? 1ui32 : 0ui32) };
        const auto tilesPerLayer { (extent.z / _tileExtent.z) + ((extent.z % _tileExtent.z) ? 1ui32 : 0ui32) };

        const auto tiles { tilesPerRow * tilesPerCol * tilesPerLayer };

        _tilesPerLayer.push_back(tiles);
    }

    //
    const auto* factory { TextureFactory::get() };

    _texture = factory->build(
        {
            _extent,
            TextureFormat::eR16Uint,
            mipLimit,
            TextureType::e2d,
            vk::ImageAspectFlagBits::eColor,
            vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst,
            vk::MemoryPropertyFlagBits::eDeviceLocal,
            vk::SharingMode::eExclusive
        }
    );

    //
    generateMarkers();

    //
    factory->buildView(_texture);
}

void RevVirtualMarkerTexture::destroy() {

    _texture.destroy();
    _tilesPerLayer.clear();
    _supportedMips = {};
    _tileExtent = {};
    _extent = {};
}

math::uivec3 RevVirtualMarkerTexture::tileExtent(const u32 level_) const noexcept {

    if (level_ < 6ui32) {
        return _tileExtent;
    }

    auto ext { math::uivec3 { _extent } >> level_ };
    ext.z = 1ui32;
    return ext;
}

u32 RevVirtualMarkerTexture::tileCount() const noexcept {

    u32 tiles { 0ui32 };
    for (const auto& entry : _tilesPerLayer) {
        tiles += entry;
    }

    return tiles;
}

u32 RevVirtualMarkerTexture::tileCount(const u32 level_) const noexcept {
    return _tilesPerLayer[level_];
}

s64 RevVirtualMarkerTexture::estimateTileDataSize(const TextureFormat format_) const noexcept {

    auto size { formatDataSize(format_) };
    if (size > 0) {
        size *= tileCount();
    }

    return size;
}

s32 RevVirtualMarkerTexture::tileIndex(const math::uivec3 uvw_, const u32 level_) const noexcept {

    auto extent { _extent >> level_ };
    extent.x = extent.x > 0ui32 ? extent.x : 1ui32;
    extent.y = extent.y > 0ui32 ? extent.y : 1ui32;
    extent.z = extent.z > 0ui32 ? extent.z : 1ui32;

    const auto tilesPerRow { (extent.x / _tileExtent.x) + ((extent.x % _tileExtent.x) ? 1ui32 : 0ui32) };
    const auto tilesPerCol { (extent.y / _tileExtent.y) + ((extent.y % _tileExtent.y) ? 1ui32 : 0ui32) };
    const auto tilesPerLayer { (extent.z / _tileExtent.z) + ((extent.z % _tileExtent.z) ? 1ui32 : 0ui32) };

    const auto result { static_cast<u16>((static_cast<u16>(level_) << internal_mip_shift) & internal_mip_mask) };

    const auto rowTile { uvw_.x / _tileExtent.x };
    const auto colTile { uvw_.y / _tileExtent.y };
    const auto layerTile { uvw_.z / _tileExtent.z };

    if (
        rowTile >= tilesPerRow ||
        colTile >= tilesPerCol ||
        layerTile >= tilesPerLayer) {
        return -1i32;
    }

    u16 idx { 0ui16 };
    // Store X-Dimension to lower 6 Bits
    idx |= rowTile;
    // Store Y-Dimension to upper 6 Bits
    idx |= (colTile << internal_uvw_bit_per_dim);

    // Warning: Z-Collapse :: Suppress third dimension
    //idx += (layerTile * tilesPerCol * tilesPerRow);

    return { result | idx };
}

std::pair<s32, math::uivec3> RevVirtualMarkerTexture::tileFromIndex(const u16 index_) const noexcept {

    const auto mip { (index_ & internal_mip_mask) >> internal_mip_shift };
    const auto dx { index_ & internal_uvw_dim_mask };
    const auto dy { (index_ >> internal_uvw_bit_per_dim) & internal_uvw_dim_mask };

    auto extent { _extent >> mip };
    extent.x = extent.x > 0ui32 ? extent.x : 1ui32;
    extent.y = extent.y > 0ui32 ? extent.y : 1ui32;
    extent.z = extent.z > 0ui32 ? extent.z : 1ui32;

    const auto tilesPerRow { (extent.x / _tileExtent.x) + ((extent.x % _tileExtent.x) ? 1ui32 : 0ui32) };
    const auto tilesPerCol { (extent.y / _tileExtent.y) + ((extent.y % _tileExtent.y) ? 1ui32 : 0ui32) };
    // Warning: Z-Collapsed :: const auto tilesPerLayer { (extent.z / _tileExtent.z) + ((extent.z % _tileExtent.z) ? 1ui32 : 0ui32) };

    if (not isSupportedMip(mip) || dx >= tilesPerRow || dy >= tilesPerCol) {
        return _STD make_pair(-1i32, math::uivec3 {});
    }

    return _STD make_pair(
        mip,
        math::uivec3 {
            dx * _tileExtent.x,
            dy * _tileExtent.y,
            // Warning: Z-Collapse :: Suppress third dimension
            0ui32
        }
    );
}

u16 tileBitOffsetToIndex(u16 offset_) {

    if (offset_ < 1ui32) {
        return static_cast<u16>((13ui16 << internal_mip_shift) & internal_mip_mask);
    }

    if (offset_ < 2ui32) {
        return static_cast<u16>((12ui16 << internal_mip_shift) & internal_mip_mask);
    }

    if (offset_ < 3ui32) {
        return static_cast<u16>((11ui16 << internal_mip_shift) & internal_mip_mask);
    }

    if (offset_ < 4ui32) {
        return static_cast<u16>((10ui16 << internal_mip_shift) & internal_mip_mask);
    }

    if (offset_ < 5ui32) {
        return static_cast<u16>((9ui16 << internal_mip_shift) & internal_mip_mask);
    }

    if (offset_ < 6ui32) {
        return static_cast<u16>((8ui16 << internal_mip_shift) & internal_mip_mask);
    }

    if (offset_ < 7ui32) {
        return static_cast<u16>((7ui16 << internal_mip_shift) & internal_mip_mask);
    }

    if (offset_ < 8ui32) {
        return static_cast<u16>((6ui16 << internal_mip_shift) & internal_mip_mask);
    }

    if (offset_ < 12ui32) {

        constexpr u16 dim { 2ui16 };
        const auto inner { offset_ - 8ui16 };

        return static_cast<u16>(
            static_cast<u16>((5ui16 << internal_mip_shift) & internal_mip_mask) |
            (inner /* (modulo) */ % dim) |
            (inner /* (div) */ / dim) << internal_uvw_bit_per_dim
        );
    }

    if (offset_ < 28ui32) {

        constexpr u16 dim { 4ui16 };
        const auto inner { offset_ - 12ui16 };

        return static_cast<u16>(
            static_cast<u16>((4ui16 << internal_mip_shift) & internal_mip_mask) |
            (inner /* (modulo) */ % dim) |
            (inner /* (div) */ / dim) << internal_uvw_bit_per_dim
        );
    }

    if (offset_ < 92ui32) {

        constexpr u16 dim { 8ui16 };
        const auto inner { offset_ - 28ui16 };

        return static_cast<u16>(
            static_cast<u16>((3ui16 << internal_mip_shift) & internal_mip_mask) |
            (inner /* (modulo) */ % dim) |
            (inner /* (div) */ / dim) << internal_uvw_bit_per_dim
        );
    }

    if (offset_ < 348ui32) {

        constexpr u16 dim { 16ui16 };
        const auto inner { offset_ - 92ui16 };

        return static_cast<u16>(
            static_cast<u16>((2ui16 << internal_mip_shift) & internal_mip_mask) |
            (inner /* (modulo) */ % dim) |
            (inner /* (div) */ / dim) << internal_uvw_bit_per_dim
        );
    }

    if (offset_ < 1372ui32) {

        constexpr u16 dim { 32ui16 };
        const auto inner { offset_ - 348ui16 };

        return static_cast<u16>(
            static_cast<u16>((1ui16 << internal_mip_shift) & internal_mip_mask) |
            (inner /* (modulo) */ % dim) |
            (inner /* (div) */ / dim) << internal_uvw_bit_per_dim
        );
    }

    constexpr u16 dim { 64ui16 };
    const auto inner { offset_ - 1372ui16 };

    return static_cast<u16>(
        static_cast<u16>((0ui16 << internal_mip_shift) & internal_mip_mask) |
        (inner /* (modulo) */ % dim) |
        (inner /* (div) */ / dim) << internal_uvw_bit_per_dim
    );
}

Vector<u16> RevVirtualMarkerTexture::tileBitToIndex(const ptr<const u32> bitmask_) noexcept {

    Vector<u16> indices {};

    /* Octree Header */
    const ptr<const uint8_t> oct {
        static_cast<const ptr<const uint8_t>>(static_cast<const ptr<const void>>(bitmask_))
    };
    if (not *oct) {
        return indices;
    }
    /**/

    /**
     * 1x1      ::  13.0/0 -> [0..1)
     * 2x2      ::  12.0/1 -> [1..2)
     * 4x4      ::  11.0/2 -> [2..3)
     * 8x8      ::  10.0/3 -> [3..4)
     * 16x16    ::  9.0/4 -> [4..5)
     * 32x32    ::  8.0/5 -> [5..6)
     * 64x64    ::  7.0/6 -> [6..7)
     * 128x128  ::  6.0/7 -> [7..8)
     * 256x256  ::  5.0/8 -> [8..12)
     * 512x512  ::  4.0/9 -> [12..28)
     * 1k x 1k  ::  3.0/10 -> [28..92)
     * 2k x 2k  ::  2.0/11 -> [92..348)
     * 4k x 4k  ::  1.0/12 -> [348..1372)
     * 8k x 8k  ::  0.0/13 -> [1372..5468)
     */

    constexpr auto decisions { 5468ui16 };
    constexpr auto patchSize { 128ui16 };
    constexpr auto childsPerNode { 8ui16 };

    // Octree - Primary level counter
    constexpr auto __ploctC__ { decisions / patchSize / childsPerNode };
    constexpr auto ploctC { (__ploctC__ * childsPerNode * patchSize >= decisions) ? __ploctC__ : __ploctC__ + 1 };

    // Octree - Second level counter
    constexpr auto sloctC { childsPerNode };

    // Octree - Decisions per Second Node
    constexpr auto dpsn { patchSize * childsPerNode };

    // Octree - Decision of Last Node
    constexpr auto doln { decisions % dpsn };
    // Octree - Last level counter
    constexpr auto lloctC { (doln % patchSize == 0) ? doln / patchSize : doln / patchSize + 1 };
    // Octree - Last patch size
    constexpr auto lps { doln - (lloctC - 1) * patchSize };

    /**
     * Tree controlled loop
     */

    #define USE_HEADER_TREE

    #ifdef USE_HEADER_TREE
    u16 outerOffset { 0ui16 };
    const ptr<const u8> cursor {
        static_cast<const ptr<const u8>>(
            static_cast<const ptr<const void>>(bitmask_ + (2ui32 /* add offset of header */))
        )
    };

    for (u16 pli { 0ui16 }; pli < ploctC; ++pli) {

        if (not (*oct & (1 << pli))) {
            outerOffset += dpsn;
            continue;
        }

        const auto slie { pli == (ploctC - 1) ? lloctC : sloctC };
        for (u16 sli { 0ui16 }; sli < slie; ++sli) {

            if (not (*(oct + 1ui16 + pli) & (1 << sli))) {
                outerOffset += patchSize;
                continue;
            }

            const auto blie { (pli == (ploctC - 1)) && (sli == (slie - 1)) ? lps : patchSize };
            for (
                u16 bit { 0ui16 };
                bit < blie;
                #ifdef __AVX2__
                bit += 8ui16
                #else
                ++bit
                #endif
            ) {

                #ifdef __AVX2__

                constexpr __m256i mask {
                    1,
                    0,
                    0,
                    0,
                    1,
                    0,
                    0,
                    0,
                    1,
                    0,
                    0,
                    0,
                    1,
                    0,
                    0,
                    0,
                    1,
                    0,
                    0,
                    0,
                    1,
                    0,
                    0,
                    0,
                    1,
                    0,
                    0,
                    0,
                    1,
                    0,
                    0,
                    0,
                };

                u32 ipdr[8] = {
                    (u32)(bit),
                    (u32)(bit + 1),
                    (u32)(bit + 2),
                    (u32)(bit + 3),
                    (u32)(bit + 4),
                    (u32)(bit + 5),
                    (u32)(bit + 6),
                    (u32)(bit + 7)
                };
                const u32 iood[8] {
                    outerOffset,
                    outerOffset,
                    outerOffset,
                    outerOffset,
                    outerOffset,
                    outerOffset,
                    outerOffset,
                    outerOffset
                };

                const __m256i ibiod = _mm256_add_epi32(*(__m256i*)&ipdr, *(__m256i*)&iood);
                const __m256i ibod = _mm256_srli_epi32(ibiod, 3);
                const __m256i iibod = _mm256_sub_epi32(
                    ibiod,
                    _mm256_slli_epi32(ibod, 3)
                );

                __m256i idcv = _mm256_i32gather_epi32(reinterpret_cast<int const*>(cursor), ibod, 1);
                idcv = _mm256_srav_epi32(idcv, iibod);
                //const __mmask8 irv = _mm256_test_epi32_mask(idcv, mask); // Warning: Requires AVX512-F
                auto irv = _mm256_and_epi32(idcv, mask);

                for (u32 i = 0; i < 8; ++i) {

                    if (irv.m256i_u32[i]) {
                        const auto index = tileBitOffsetToIndex(ibiod.m256i_u32[i]);
                        indices.push_back(index);
                    }

                }

                #else

                // Optimize (Nr.1) :: Use const bit before byte conversion

                const auto bitOffset { outerOffset + bit };
                const auto byteOffset { bitOffset / 8ui16 };
                /* Attention: `(byteOffset * 8ui16)` can not be replaced, cause of int 'div' dropping 'rest' */
                const auto innerBitOffset { bitOffset - (byteOffset * 8ui16) };

                if (*(cursor + byteOffset) >> innerBitOffset & 0b1) {
                    const auto index = tileBitOffsetToIndex(static_cast<u16>(bitOffset));
                    indices.push_back(index);
                }

                #endif
            }

            outerOffset += blie;
        }

    }

    #else

    u16 outerOffset { 0ui16 };
    auto* cursor { bitmask_ + (2ui32 /* add offset of header */) };
    while (outerOffset < 5468ui16) {

        for (u16 bit { 0ui16 }; bit < (sizeof(u32) * 8ui16); ++bit) {

            if ((*cursor >> bit) & 0b1) {
                const auto index = tileBitOffsetToIndex(outerOffset + bit);
                indices.push_back(index);
            }
        }

        outerOffset += (sizeof(u32) * 8ui16);
        ++cursor;
    }
    #endif

    #undef USE_HEADER_TREE

    return indices;
}

bool RevVirtualMarkerTexture::isSupportedMip(const u32 level_) const noexcept {
    return _supportedMips.min <= level_ && level_ < _supportedMips.max;
}

cref<Texture> RevVirtualMarkerTexture::texture() const noexcept {
    return _texture;
}
