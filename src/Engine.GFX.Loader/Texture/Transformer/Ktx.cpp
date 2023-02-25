#include "Ktx.hpp"

#include <filesystem>
#include <gli/gli.hpp>
#include <Engine.GFX/API/VkTranslate.hpp>
#include <Engine.Common/Math/Coordinates.hpp>
#include <Engine.Logging/Logger.hpp>
#include <Engine.GFX/Command/CommandBuffer.hpp>
#include <Engine.GFX/Texture/TextureFactory.hpp>
#include <Engine.GFX/Device/Device.hpp>
#include <Engine.GFX/Texture/VirtualTextureView.hpp>
#include <Engine.GFX/Texture/VirtualTexturePage.hpp>

#include "Engine.Common/Url.hpp"

using namespace ember::engine::gfx::loader::transformer;
using namespace ember::engine::gfx::loader;
using namespace ember::engine::gfx;
using namespace ember;

/**/
#pragma region KTX Format Specification
struct InternalKtxHeader {
    // Meta
    u32 vkFormat;
    u32 typeSize;
    u32 pixelWidth;
    u32 pixelHeight;
    u32 pixelDepth;
    u32 layerCount;
    u32 faceCount;
    u32 levelCount;
    u32 ss;

    // Indices
    u32 dfdOff;
    u32 dfdSize;
    u32 kvdOff;
    u32 kvdSize;
    u32 sgdOff;
    u32 sgdSize;
};

struct InternalKtxLevel {
    u64 byteOff;
    u64 byteSize;
    u64 ucByteSize;
};

static constexpr unsigned char ktx20Identifier[] = {
    0xAB,
    0x4B,
    0x54,
    0x58,
    0x20,
    0x32,
    0x30,
    0xBB,
    0x0D,
    0x0A,
    0x1A,
    0x0A
};
#pragma endregion
/**/

void transformer::convertKtx(
    const non_owning_rptr<const assets::Texture> asset_,
    cref<smr<resource::Source>> src_,
    const non_owning_rptr<VirtualTextureView> dst_,
    cref<sptr<Device>> device_,
    const TextureLoadOptions options_
) {

    constexpr auto chunkSize = MAX(sizeof(gli::detail::FOURCC_KTX10), sizeof(ktx20Identifier));
    Vector<unsigned char> raw {};
    raw.resize(chunkSize);

    streamsize bytes {};
    src_->get(0, chunkSize, raw.data(), bytes);

    assert(bytes >= chunkSize);

    /**/

    bool isKtx20 = true;
    for (size_t idx = 0; idx < sizeof(ktx20Identifier); ++idx) {
        if (raw[idx] != ktx20Identifier[idx]) {
            isKtx20 = false;
            break;
        }
    }

    if (isKtx20) {
        convertKtx20(asset_, src_, dst_, device_, options_);
        return;
    }

    /**/

    bool isKtx10 = true;
    for (size_t idx = 0; idx < sizeof(gli::detail::FOURCC_KTX10); ++idx) {
        if (raw[idx] != gli::detail::FOURCC_KTX10[idx]) {
            isKtx10 = false;
            break;
        }
    }

    if (isKtx10) {
        convertKtx10Gli(asset_, src_, dst_, device_, options_);
        return;
    }

    /**/

    #ifdef _DEBUG
    Logger::warn(
        R"(Tried to convert binary data of `{}` as KTX into texture but got no valid identifier.)",
        asset_->getAssetName()
    );
    #endif
}

void transformer::convertKtxPartial(
    const non_owning_rptr<const assets::Texture> asset_,
    cref<smr<resource::Source>> src_,
    const non_owning_rptr<VirtualTextureView> dst_,
    cref<sptr<Device>> device_,
    const TextureStreamOptions options_
) {

    if (options_.op == TextureStreamOp::eUnload) {

        /**
         * Unload targeted segment and return
         */
        unloadPartialTmp(asset_, src_, dst_, device_, options_);
        return;
    }

    /**
     * Load the targeted segment
     */
    convertKtx20Partial(asset_, src_, dst_, device_, options_);
}

/**/

#define IS_LOCKED_SEGMENT (false)

/**/

void deduceFromFormat(cref<gli::format> format_, ref<vk::Format> vkFormat_, ref<vk::ImageAspectFlags> aspect_) {

    switch (format_) {
        /**
         * R Formats (2D Images)
         *
         * Used for alpha, roughness, displacement
         */
        case gli::FORMAT_R16_SFLOAT_PACK16: {
            vkFormat_ = vk::Format::eR16Sfloat;
            aspect_ = vk::ImageAspectFlagBits::eColor;
            break;
        }
        case gli::FORMAT_R32_SFLOAT_PACK32: {
            vkFormat_ = vk::Format::eR32Sfloat;
            aspect_ = vk::ImageAspectFlagBits::eColor;
            break;
        }

        /**
         * D Formats (2D Images)
         *
         * Used for depth images or possible for alpha blending
         */
        case gli::FORMAT_D16_UNORM_PACK16: {
            vkFormat_ = vk::Format::eD16Unorm;
            aspect_ = vk::ImageAspectFlagBits::eDepth;
            break;
        }
        case gli::FORMAT_D32_SFLOAT_PACK32: {
            vkFormat_ = vk::Format::eD32Sfloat;
            aspect_ = vk::ImageAspectFlagBits::eDepth;
            break;
        }

        /**
         * DS Formats (2D Images)
         *
         * Used for depth stencil images
         */
        case gli::FORMAT_D16_UNORM_S8_UINT_PACK32: {
            vkFormat_ = vk::Format::eD16UnormS8Uint;
            aspect_ = vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil;
            break;
        }
        case gli::FORMAT_D32_SFLOAT_S8_UINT_PACK64: {
            vkFormat_ = vk::Format::eD32SfloatS8Uint;
            aspect_ = vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil;
            break;
        }

        /**
         * RGB Formats (2D Images)
         *
         * Used for color textures like albedo, sample maps or general image
         */
        case gli::FORMAT_RGB8_UNORM_PACK8: {
            vkFormat_ = vk::Format::eR8G8B8Unorm;
            aspect_ = vk::ImageAspectFlagBits::eColor;
            break;
        }
        case gli::FORMAT_RGB8_SNORM_PACK8: {
            vkFormat_ = vk::Format::eR8G8B8Snorm;
            aspect_ = vk::ImageAspectFlagBits::eColor;
            break;
        }
        case gli::FORMAT_RGB16_UNORM_PACK16: {
            vkFormat_ = vk::Format::eR16G16B16A16Unorm;
            aspect_ = vk::ImageAspectFlagBits::eColor;
            break;
        }
        case gli::FORMAT_RGB32_UINT_PACK32: {
            vkFormat_ = vk::Format::eR32G32B32Uint;
            aspect_ = vk::ImageAspectFlagBits::eColor;
            break;
        }
        case gli::FORMAT_RGB32_SFLOAT_PACK32: {
            vkFormat_ = vk::Format::eR32G32B32Sfloat;
            aspect_ = vk::ImageAspectFlagBits::eColor;
            break;
        }

        /**
        * RGBA Formats (2D Images)
        *
        * Used for color textures with alpha like albedo + blending or sample maps
        */
        case gli::FORMAT_RGBA8_UNORM_PACK8: {
            vkFormat_ = vk::Format::eR8G8B8A8Unorm;
            aspect_ = vk::ImageAspectFlagBits::eColor;
            break;
        }
        case gli::FORMAT_RGBA8_SRGB_PACK8: {
            vkFormat_ = vk::Format::eR8G8B8A8Srgb;
            aspect_ = vk::ImageAspectFlagBits::eColor;
            break;
        }
        case gli::FORMAT_RGBA8_SNORM_PACK8: {
            vkFormat_ = vk::Format::eR8G8B8A8Snorm;
            aspect_ = vk::ImageAspectFlagBits::eColor;
            break;
        }
        case gli::FORMAT_RGBA16_UNORM_PACK16: {
            vkFormat_ = vk::Format::eR16G16B16A16Unorm;
            aspect_ = vk::ImageAspectFlagBits::eColor;
            break;
        }
        case gli::FORMAT_RGBA16_SFLOAT_PACK16: {
            vkFormat_ = vk::Format::eR16G16B16A16Sfloat;
            aspect_ = vk::ImageAspectFlagBits::eColor;
            break;
        }
        case gli::FORMAT_RGBA32_UINT_PACK32: {
            vkFormat_ = vk::Format::eR32G32B32A32Uint;
            aspect_ = vk::ImageAspectFlagBits::eColor;
            break;
        }
        case gli::FORMAT_RGBA32_SFLOAT_PACK32: {
            vkFormat_ = vk::Format::eR32G32B32A32Sfloat;
            aspect_ = vk::ImageAspectFlagBits::eColor;
            break;
        }

        /**
        * RGB(A) Formats (Compressed Cube Images)
        */
        case gli::FORMAT_RGBA_ASTC_8X8_UNORM_BLOCK16: {
            vkFormat_ = vk::Format::eAstc8x8UnormBlock;
            aspect_ = vk::ImageAspectFlagBits::eColor;
            break;
        }
        case gli::FORMAT_RGB_ETC2_UNORM_BLOCK8: {
            vkFormat_ = vk::Format::eEtc2R8G8B8UnormBlock;
            aspect_ = vk::ImageAspectFlagBits::eColor;
            break;
        }
        case gli::FORMAT_RGBA_DXT5_UNORM_BLOCK16: {
            // TODO: Temporary
            vkFormat_ = vk::Format::eBc2UnormBlock;
            aspect_ = vk::ImageAspectFlagBits::eColor;
            break;
        }
        default: {
            throw _STD exception("Unresolved texture format.");
        }
    }
}

void deduceFromFormat(cref<vk::Format> format_, ref<vk::Format> vkFormat_, ref<vk::ImageAspectFlags> aspect_) {

    auto emberFormat { api::vkTranslateFormat(format_) };

    if (isDepthStencilFormat(emberFormat)) {
        aspect_ = vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil;
    } else if (isDepthFormat(emberFormat)) {
        aspect_ = vk::ImageAspectFlagBits::eDepth;
    } else if (isStencilFormat(emberFormat)) {
        aspect_ = vk::ImageAspectFlagBits::eStencil;
    } else {
        aspect_ = vk::ImageAspectFlagBits::eColor;
    }

    vkFormat_ = format_;
}

void transformer::convertKtx10Gli(
    const non_owning_rptr<const engine::assets::Texture> asset_,
    cref<smr<engine::resource::Source>> src_,
    const non_owning_rptr<VirtualTextureView> dst_,
    cref<sptr<Device>> device_,
    const TextureLoadOptions options_
) {

    const auto srcSize = src_->size();
    assert(srcSize > 0);

    Vector<char> tmp {};
    tmp.resize(srcSize);

    streamsize bytes {};
    src_->get(0, srcSize, tmp.data(), bytes);

    /**/

    assert(bytes >= srcSize);
    gli::texture glitex = gli::load_ktx(tmp.data(), bytes);

    /**/

    const auto lvlZeroExt = glitex.extent(0);
    math::ivec3 extent = {
        lvlZeroExt.x,
        lvlZeroExt.y,
        lvlZeroExt.z
    };

    const auto lvlZeroForm = glitex.format();
    vk::Format format;
    vk::ImageAspectFlags aspect;
    vk::ImageCreateFlags create = vk::ImageCreateFlags();
    vk::ImageUsageFlags usage = vk::ImageUsageFlagBits::eTransferSrc |
        vk::ImageUsageFlagBits::eTransferDst |
        vk::ImageUsageFlagBits::eSampled;

    deduceFromFormat(lvlZeroForm, format, aspect);

    const auto faceCount = glitex.faces();
    if (faceCount == 6) {
        create = vk::ImageCreateFlagBits::eCubeCompatible;
    }

    /* Validate Matching Descriptor */
    #ifdef _DEBUG

    if (dst_->format() != api::vkTranslateFormat(format)) {
        __debugbreak();
    }

    if (dst_->mipLevels() < glitex.levels()) {
        __debugbreak();
    }

    if (create & vk::ImageCreateFlagBits::eCubeCompatible && dst_->type() != TextureType::eCube) {
        __debugbreak();
    }

    // Validate Aspect Flag Bits
    // Validate Usage Flag Bits

    #endif

    /**
     * Staging Buffer
     */
    Buffer stage {};

    vk::BufferCreateInfo bci {
        vk::BufferCreateFlags(),
        glitex.size(),
        vk::BufferUsageFlagBits::eTransferSrc,
        vk::SharingMode::eExclusive,
        0,
        nullptr
    };

    stage.buffer = device_->vkDevice().createBuffer(bci);
    stage.device = device_->vkDevice();

    const auto allocResult {
        memory::allocate(
            device_->allocator(),
            device_,
            stage.buffer,
            MemoryProperties { MemoryProperty::eHostVisible } | MemoryProperty::eHostCoherent,
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
    stage.map();
    assert(stage.memory->mapping);

    /**
     * Copy Data
     */
    stage.write(glitex.data(), glitex.size());

    /**
     *
     */
    stage.unmap();

    /**
     * Fetch Region per Layer
     */

    const auto minMipLevel = _STD max(dst_->minMipLevel(), static_cast<u32>(glitex.base_level()));
    const auto maxMipLevel = _STD min(dst_->maxMipLevel(), static_cast<u32>(glitex.max_level()));

    /**/

    // Prevent signed to unsigned wrapping (-1 ~> ~0)
    assert(maxMipLevel <= dst_->maxMipLevel());
    assert(minMipLevel <= maxMipLevel);

    /**/

    Vector<vk::BufferImageCopy> regions {};
    uint32_t offset = 0;

    if (create & vk::ImageCreateFlagBits::eCubeCompatible) {

        const gli::texture_cube ct { glitex };

        assert(dst_->depth() == 1ui32);

        for (uint32_t face = 0; face < glitex.faces(); face++) {
            for (uint32_t level = minMipLevel; level <= maxMipLevel; ++level) {

                const auto se = ct[face][level].extent();

                vk::BufferImageCopy copy {
                    offset,
                    0,
                    0,
                    {
                        aspect,
                        level,
                        face,
                        1ui32
                    },
                    vk::Offset3D(),
                    vk::Extent3D {
                        static_cast<uint32_t>(se.x),
                        static_cast<uint32_t>(se.y),
                        1ui32
                    }
                };

                regions.push_back(copy);
                offset += static_cast<uint32_t>(ct[face][level].size());
            }
        }

    } else {
        /**
         * Copy Buffer Image (2D / 2D Array)
         */
        for (uint32_t level = minMipLevel; level <= maxMipLevel; ++level) {
            vk::BufferImageCopy copy {
                offset,
                0,
                0,
                {
                    aspect,
                    level,
                    0,
                    dst_->type() == TextureType::e2dArray ? dst_->depth() : 1
                },
                vk::Offset3D(),
                vk::Extent3D {
                    dst_->width() / (0x1 << level),
                    dst_->height() / (0x1 << level),
                    dst_->type() == TextureType::e2dArray ? 1 : dst_->depth()
                }
            };

            regions.push_back(copy);
            offset += static_cast<uint32_t>(glitex.size(level));
        }
    }

    /**
     * Ensure virtual memory is residential
     */
    for (auto& page : dst_->pages()) {

        if (IS_LOCKED_SEGMENT) {
            // TODO: Lock effected memory and texture pages!
        }

        if (page->memory()->state() != VirtualMemoryPageState::eLoaded) {
            dst_->owner()->load(page);
        }

    }

    /**
    * Update virtual binding data
    */
    const auto* tex { dst_->owner() };
    const_cast<VirtualTexture*>(tex)->updateBindingData();
    #pragma warning(push)
    #pragma warning(disable: 4996)
    const_cast<VirtualTexture*>(tex)->enqueueBindingSync(device_->graphicsQueue());
    #pragma warning(pop)

    /**
     * Capture commands to copy data to image
     */
    auto pool = device_->transferQueue()->pool();
    pool->lck().acquire();
    CommandBuffer cmd = pool->make();
    cmd.begin();

    /**
     * Pre-Transform image layout
     */

    vk::ImageMemoryBarrier simb {
        vk::AccessFlags(),
        vk::AccessFlags(),
        vk::ImageLayout::eUndefined,
        vk::ImageLayout::eTransferDstOptimal,
        VK_QUEUE_FAMILY_IGNORED,
        VK_QUEUE_FAMILY_IGNORED,
        dst_->owner()->vkImage(),
        {
            aspect,
            0,
            (maxMipLevel - minMipLevel) + 1,
            dst_->baseLayer(),
            dst_->layers()
        }
    };

    cmd.vkCommandBuffer().pipelineBarrier(
        vk::PipelineStageFlagBits::eAllCommands,
        vk::PipelineStageFlagBits::eTransfer,
        vk::DependencyFlags(),
        0,
        nullptr,
        0,
        nullptr,
        1,
        &simb
    );

    /**
     * Copy data from buffer to image
     */

    cmd.vkCommandBuffer().copyBufferToImage(
        stage.buffer,
        dst_->owner()->vkImage(),
        vk::ImageLayout::eTransferDstOptimal,
        static_cast<uint32_t>(regions.size()),
        regions.data()
    );

    /**
     * Restore Layout
     */
    vk::ImageMemoryBarrier eimb {
        vk::AccessFlags(),
        vk::AccessFlags(),
        vk::ImageLayout::eTransferDstOptimal,
        vk::ImageLayout::eShaderReadOnlyOptimal,
        VK_QUEUE_FAMILY_IGNORED,
        VK_QUEUE_FAMILY_IGNORED,
        dst_->owner()->vkImage(),
        {
            aspect,
            0,
            (maxMipLevel - minMipLevel) + 1,
            dst_->baseLayer(),
            dst_->layers()
        }
    };

    cmd.vkCommandBuffer().pipelineBarrier(
        vk::PipelineStageFlagBits::eTransfer,
        vk::PipelineStageFlagBits::eAllCommands,
        vk::DependencyFlags(),
        0,
        nullptr,
        0,
        nullptr,
        1,
        &eimb
    );

    /**
     * Dispatch commands
     */

    cmd.end();
    cmd.submitWait();
    cmd.release();

    pool->lck().release();

    /**
     * Cleanup
     */
    stage.destroy();
    glitex.clear();

    tmp.clear();
}

void transformer::convertKtx20(
    const non_owning_rptr<const engine::assets::Texture> asset_,
    cref<smr<engine::resource::Source>> src_,
    const non_owning_rptr<VirtualTextureView> dst_,
    cref<sptr<Device>> device_,
    const TextureLoadOptions options_
) {

    /**/
    auto dst { _STD move(dst_) };
    assert(
        dst->type() == TextureType::e2d ||
        dst->type() == TextureType::e2dArray ||
        (dst->type() == TextureType::eCube && not (options_.dataFlag & TextureLoadDataFlagBits::eLazyDataLoading))
    );
    /**/

    Vector<char> tmp {};
    gli::texture glitex {};

    /**/
    math::ivec3 extent {};

    vk::Format format;
    vk::ImageAspectFlags aspect;

    u32 effectedMipLevels { 0ui32 };
    const u32 layerCount { dst->type() == TextureType::eCube ? 6ui32 : 1ui32 };
    /**/

    if (options_.dataFlag == TextureLoadDataFlagBits::eLazyDataLoading) {
        /**
         * Just load header and meta data
         *
         * @see gli::load(...) load.inl#L9
         */
        constexpr auto header_min_size { sizeof(InternalKtxHeader) };

        constexpr auto chunkSize { sizeof(ktx20Identifier) + header_min_size };
        _STD vector<char> raw(static_cast<size_t>(chunkSize));

        streamsize bytes {};
        src_->get(0, chunkSize, raw.data(), bytes);

        assert(bytes >= chunkSize);

        auto* cursor { raw.data() + sizeof(ktx20Identifier) };
        cref<InternalKtxHeader> header { *reinterpret_cast<ptr<const InternalKtxHeader>>(cursor) };

        // Extent
        extent = {
            static_cast<s32>(header.pixelWidth),
            _STD max(static_cast<s32>(header.pixelHeight), 1i32),
            _STD max(static_cast<s32>(header.pixelDepth), 1i32)
        };

        // Format
        deduceFromFormat(*reinterpret_cast<const vk::Format*>(&header.vkFormat), format, aspect);

        // Mip Levels
        effectedMipLevels = _STD min(header.levelCount, dst->mipLevels());

    } else {

        /**
         * Fully load source file
         */
        constexpr auto header_min_size { sizeof(gli::detail::ktx_header10) };

        const auto chunkSize { sizeof(gli::detail::FOURCC_KTX10) + header_min_size };
        _STD vector<char> raw(static_cast<size_t>(chunkSize));

        streamsize bytes {};
        src_->get(0, chunkSize, raw.data(), bytes);

        assert(bytes >= chunkSize);

        auto* cursor { raw.data() + sizeof(gli::detail::FOURCC_KTX10) };
        cref<gli::detail::ktx_header10> header {
            *reinterpret_cast<ptr<const gli::detail::ktx_header10>>(cursor)
        };

        /**/

        const auto srcSize = src_->size();
        assert(srcSize > 0);

        tmp.resize(srcSize);

        src_->get(0, srcSize, tmp.data(), bytes);

        /**/

        assert(bytes >= srcSize);
        glitex = gli::load_ktx(tmp.data(), bytes);

        // Extent
        const auto lvlZeroExt = glitex.extent(0);
        extent = {
            lvlZeroExt.x,
            lvlZeroExt.y,
            lvlZeroExt.z
        };

        // Format
        const auto lvlZeroForm = glitex.format();
        deduceFromFormat(lvlZeroForm, format, aspect);

        // Mip Levels
        const_cast<ref<u32>>(effectedMipLevels) = _STD min(static_cast<u32>(glitex.levels()), dst->mipLevels());
    }

    /**/
    assert(dst->width() == extent.x);
    assert(dst->height() == extent.y);
    assert(dst->depth() == extent.z);
    /**/

    /**/
    assert(dst->format() == api::vkTranslateFormat(format));
    /**/

    /**
    * Staging Buffer
    */
    Buffer stage {};

    Vector<vk::BufferImageCopy> regions {};

    if (options_.dataFlag != TextureLoadDataFlagBits::eLazyDataLoading) {

        /**
         * Setup vulkan stage buffer to eager load texture
         */
        vk::BufferCreateInfo bci {
            vk::BufferCreateFlags(),
            MAX(glitex.size(), 128ui64),
            vk::BufferUsageFlagBits::eTransferSrc,
            vk::SharingMode::eExclusive,
            0,
            nullptr
        };

        stage.buffer = device_->vkDevice().createBuffer(bci);
        stage.device = device_->vkDevice();

        const auto allocResult {
            memory::allocate(
                device_->allocator(),
                device_,
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
         * Copy Data
         */
        stage.write(glitex.data(), glitex.size());
        stage.flushAligned(glitex.size());

        /**
         *
         */
        stage.unmap();

        /**
         * Fetch Region per Layer
         */
        uint32_t offset = 0;

        if (dst->type() == TextureType::eCube) {

            const gli::texture_cube ct { glitex };

            for (uint32_t face = 0; face < glitex.faces(); face++) {
                for (uint32_t level = 0; level < dst->mipLevels(); ++level) {

                    const auto se = ct[face][level].extent();

                    vk::BufferImageCopy copy {
                        offset,
                        0,
                        0,
                        {
                            aspect,
                            level,
                            face,
                            1ui32
                        },
                        vk::Offset3D(),
                        vk::Extent3D {
                            static_cast<uint32_t>(se.x),
                            static_cast<uint32_t>(se.y),
                            1ui32
                        }
                    };

                    regions.push_back(copy);
                    offset += static_cast<uint32_t>(ct[face][level].size());
                }
            }

        } else {
            /**
             * Copy Buffer Image (2D / 2D Array)
             */
            for (uint32_t level = 0; level < dst->mipLevels(); ++level) {
                vk::BufferImageCopy copy {
                    offset,
                    0,
                    0,
                    {
                        aspect,
                        level,
                        0,
                        dst->type() == TextureType::e2dArray ? dst->depth() : 1
                    },
                    vk::Offset3D(),
                    vk::Extent3D {
                        dst->width() / (0x1 << level),
                        dst->height() / (0x1 << level),
                        dst->type() == TextureType::e2dArray ? 1 : dst->depth()
                    }
                };

                regions.push_back(copy);
                offset += static_cast<uint32_t>(glitex.size(level));
            }
        }

        // Warning: Temporary
        for (auto* page : dst->pages()) {

            if (IS_LOCKED_SEGMENT) {
                // TODO: Lock effected memory and texture pages!!!
            }

            if (page->memory()->state() != VirtualMemoryPageState::eLoaded) {
                dst->owner()->load(page);
            }
        }
    }

    /**
    * Update virtual binding data
    */
    const auto* tex { dst->owner() };
    const_cast<VirtualTexture*>(tex)->updateBindingData();
    #pragma warning(push)
    #pragma warning(disable: 4996)
    const_cast<VirtualTexture*>(tex)->enqueueBindingSync(device_->graphicsQueue());
    #pragma warning(pop)

    /**
    * Transform and data transfer
    */
    auto pool = device_->transferQueue()->pool();
    pool->lck().acquire();
    CommandBuffer cmd = pool->make();
    cmd.begin();

    /**/
    vk::ImageMemoryBarrier preBarrier {};
    vk::ImageMemoryBarrier postBarrier {};
    /**/

    if (options_.dataFlag != TextureLoadDataFlagBits::eLazyDataLoading) {
        /**
         * Copy Data to Image
         */
        preBarrier = {
            vk::AccessFlags(),
            vk::AccessFlags(),
            vk::ImageLayout::eUndefined,
            vk::ImageLayout::eTransferDstOptimal,
            VK_QUEUE_FAMILY_IGNORED,
            VK_QUEUE_FAMILY_IGNORED,
            dst->owner()->vkImage(),
            {
                aspect,
                0,
                effectedMipLevels,
                dst->baseLayer(),
                layerCount
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

        cmd.vkCommandBuffer().copyBufferToImage(
            stage.buffer,
            dst->owner()->vkImage(),
            vk::ImageLayout::eTransferDstOptimal,
            static_cast<u32>(regions.size()),
            regions.data()
        );
    }

    /**
    * Restore Layout
    */
    postBarrier = {
        vk::AccessFlags(),
        vk::AccessFlags(),
        (options_.dataFlag & TextureLoadDataFlagBits::eLazyDataLoading) ?
            vk::ImageLayout::eUndefined :
            vk::ImageLayout::eTransferDstOptimal,
        vk::ImageLayout::eShaderReadOnlyOptimal,
        VK_QUEUE_FAMILY_IGNORED,
        VK_QUEUE_FAMILY_IGNORED,
        dst->owner()->vkImage(),
        {
            aspect,
            0,
            effectedMipLevels,
            dst->baseLayer(),
            layerCount
        }
    };

    cmd.vkCommandBuffer().pipelineBarrier(
        vk::PipelineStageFlagBits::eTransfer,
        vk::PipelineStageFlagBits::eAllGraphics,
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
    * Cleanup
    */
    if (options_.dataFlag != TextureLoadDataFlagBits::eLazyDataLoading) {
        stage.destroy();
        glitex.clear();
    }
}

void transformer::convertKtx20Partial(
    const non_owning_rptr<const engine::assets::Texture> asset_,
    cref<smr<engine::resource::Source>> src_,
    const non_owning_rptr<VirtualTextureView> dst_,
    cref<sptr<Device>> device_,
    const TextureStreamOptions options_
) {

    /**
    *
    */
    math::uivec3 reqExtent { options_.extent };
    math::uivec3 reqOffset { options_.offset };

    /**
    * Check for memory changes to prevent excessive file loading
    */
    #pragma region Preserve Memory Changes

    /* (Optimization I :: Binary Search Begin) */
    auto pageIter { dst_->pages().begin() };

    if (options_.mip > 0) {
        // We assume that the pages of each texture have a geometric distribution, therefore we might skip the first subset
        // Warning: Only when using 8k Textures :: 8192 / 128 ~> 64 ~> 4096 Pages

        auto estSkip { 0ui64 };
        if (dst_->pages().size() >= 5200ui64) {
            estSkip = 4095ui64;
        }

        const auto flbe {
            _STD ranges::lower_bound(
                dst_->pages().begin() + estSkip,
                dst_->pages().end(),
                options_.mip,
                _STD ranges::less {},
                [](non_owning_rptr<VirtualTexturePage> entry_) {
                    return entry_->mipLevel();
                }
            )
        };

        pageIter += _STD distance(dst_->pages().begin(), flbe);
    }
    /**/

    u32 mipOff { static_cast<u32>(_STD distance(dst_->pages().begin(), pageIter)) };

    bool changedMemory { false };

    if (mipOff >= dst_->pages().size()) {
        mipOff = dst_->pages().size() - 1;
    }

    if (dst_->pages()[mipOff]->flags() & VirtualTexturePageFlag::eOpaqueTail) {

        changedMemory = dst_->pages()[mipOff]->memory()->state() != VirtualMemoryPageState::eLoaded;

        if (changedMemory) {
            dst_->owner()->load(dst_->pages()[mipOff]);
        }
    }

    if (options_.mip < dst_->owner()->mipTailFirstLod()) {

        //constexpr auto pageExt { 128ui32 };
        const math::uivec3 pageExt { dst_->owner()->granularity() };

        const math::uivec3 acc {
            math::compMax<math::uivec3::value_type>(
                (dst_->extent() >> options_.mip) / pageExt,
                math::uivec3 { 1ui32 }
            )
        };
        const math::uivec3 off { options_.offset / pageExt };
        const math::uivec3 ext {
            math::compMax<math::uivec3::value_type>(options_.extent / pageExt, math::uivec3 { 1ui32 })
        };

        /**
         * Reexpand the ext to the requested size to apply page granularity for data loading
         */
        reqExtent = ext * pageExt;

        /**
         * Reexpand tthe off to tthe requested offset to apply page granularity for data loading
         */
        reqOffset = off * pageExt;

        u32 oi { off.x + off.y * acc.x };
        const auto out { (off.x + ext.x) + (off.y + ext.y) * acc.x };

        while (oi < out) {
            for (u32 ix { 0ui32 }; ix < ext.x; ++ix) {

                u32 idx { mipOff + oi + ix };
                auto* page { dst_->pages()[idx] };

                bool effected { false };
                if (page->mipLevel() != options_.mip) {
                    continue;

                }

                const auto minPage { page->offset() };
                const auto maxPage { page->offset() + page->extent() };

                const auto minDst { reqOffset };
                const auto maxDst { reqOffset + reqExtent };

                const auto pX { minPage.x >= maxDst.x || maxPage.x <= minDst.x };
                const auto pY { minPage.y >= maxDst.y || maxPage.y <= minDst.y };
                const auto pZ { minPage.z >= maxDst.z || maxPage.z <= minDst.z };

                effected = !(pX || pY || pZ);

                if (effected && page->memory()->state() != VirtualMemoryPageState::eLoaded) {
                    dst_->owner()->load(page);
                    changedMemory = true;

                }

            }
            oi += acc.x;

        }
    }

    #if FALSE
    /* (Optimization II :: Early-Exit Iterator Loop) */
    bool changedMemory{ false };
    for (; pageIter != dst_->pages().end() && (*pageIter)->mipLevel() == options_.mip; ++pageIter) {

        auto* page{ *pageIter };

        if (page->layer() != options_.layer) {
            continue;
        }

        // Assuming the pages are sorted by there mip and linearized as grid, we can compute some `bounding box` to check
        //bool testLower { false };
        bool testUpper{ false };

        /*
         *              |                   |
         *   Offset     |   Y-Skip (ax*dy)  |
         *              |                   |
         *  ------------|-------------------|-------
         *              |                   |
         *  X-Skip (dx) |   Extent          |   X-Break (ix >= ox+ex)
         *              |                   |
         *  ------------|-------------------|-------
         *              |                   |
         *              |   Y-Break (iy >= oy+ey) | Exit
         *              |                   |
         */

    #if FALSE
        constexpr auto pageExt{ 128ui32 };
        const math::uivec3 acc{ (dst_->width() >> options_.mip) / pageExt };
        const math::uivec3 off{ options_.offset / pageExt };
        const math::uivec3 ext{ options_.extent / pageExt };

        u32 oi{ off.x + off.y * acc.x };
        const auto out{ (off.x + ext.x) + (off.y + ext.y) * acc.x };

        while (oi < out) {
            for (u32 ix{ 0ui32 }; ix < ext.x; ++ix) {
                u32 idx{ oi + ix };
            }
            oi += acc.x;
        }
    #endif

        bool effected{ false };
        if (page->flags() & VirtualTexturePageFlag::eOpaqueTail) {
            effected = page->mipLevel() <= options_.mip;

        }
        else {

            if (page->mipLevel() != options_.mip) {
                continue;
            }

            const auto minPage{ page->offset() };
            const auto maxPage{ page->offset() + page->extent() };

            const auto minDst{ options_.offset };
            const auto maxDst{ options_.offset + options_.extent };

            const auto pX{ minPage.x >= maxDst.x || maxPage.x <= minDst.x };
            const auto pY{ minPage.y >= maxDst.y || maxPage.y <= minDst.y };
            const auto pZ{ minPage.z >= maxDst.z || maxPage.z <= minDst.z };

            effected = !(pX || pY || pZ);

            // Upper `bounding box` check for early exit
            if (maxPage.x >= maxDst.x && maxPage.y >= maxDst.y && maxPage.z >= maxDst.z) {
                testUpper = true;
            }
        }

        if (effected && page->memory()->state() != VirtualMemoryPageState::eLoaded) {
            dst_->owner()->load(page);
            changedMemory = true;
        }

        // Early-Exit on last effected element
        if (testUpper) {
            break;
        }
    }
    #endif

    // TODO: !!! Move dirty flagging and update tracking into virtual texture itself !!!
    if (changedMemory) {
        const auto* tex { dst_->owner() };
        const_cast<VirtualTexture*>(tex)->updateBindingData();
        #pragma warning(push)
        #pragma warning(disable: 4996)
        const_cast<VirtualTexture*>(tex)->enqueueBindingSync(device_->transferQueue());
        #pragma warning(pop)
    }

    #pragma endregion

    /**
    *
    */
    if (!changedMemory && not (options_.mip >= dst_->owner()->mipTailFirstLod())) {
        return;
    }

    /**
    *
    */

    #pragma region GLI Header

    constexpr auto header_min_size { sizeof(InternalKtxHeader) };

    constexpr auto chunkSize {
        sizeof(ktx20Identifier) + header_min_size + 14 /* 14 Mips ~> 16k x 16k */ * sizeof(InternalKtxLevel)
    };
    _STD vector<char> raw(static_cast<size_t>(chunkSize));

    streamsize bytes {};
    src_->get(0, chunkSize, raw.data(), bytes);

    assert(bytes >= chunkSize);
    assert(memcmp(raw.data(), ktx20Identifier, sizeof(ktx20Identifier)) == 0);

    auto* cursor { raw.data() + sizeof(ktx20Identifier) };
    cref<InternalKtxHeader> header { *reinterpret_cast<ptr<const InternalKtxHeader>>(cursor) };

    Vector<InternalKtxLevel> indexedLevels {};
    indexedLevels.resize(header.levelCount);

    constexpr u64 offset { 68ui64 };
    memcpy(indexedLevels.data(), cursor + offset, sizeof(InternalKtxLevel) * header.levelCount);

    // Extent
    const math::uivec3 srcExtent = {
        header.pixelWidth,
        _STD max(header.pixelHeight, 1ui32),
        _STD max(header.pixelDepth, 1ui32)
    };

    // Format
    vk::Format format {};
    vk::ImageAspectFlags aspect {};
    deduceFromFormat(*reinterpret_cast<const vk::Format*>(&header.vkFormat), format, aspect);

    // Meta
    const u32 srcLevels { header.levelCount };
    const u32 srcLayers { header.layerCount };
    const u32 srcFaces { header.faceCount };

    #pragma endregion

    auto mipDiff { 0i32 };

    const auto* const dstOrigin { dst_->owner() };
    math::uivec3 mipTestExtent {
        dstOrigin->width(),
        dstOrigin->height(),
        dstOrigin->depth()
    };

    if (srcExtent.x >= mipTestExtent.x || srcExtent.y >= mipTestExtent.y) {
        while (srcExtent.x > mipTestExtent.x || srcExtent.y > mipTestExtent.y) {
            mipTestExtent.x >>= 1ui32;
            mipTestExtent.y >>= 1ui32;
            ++mipDiff;
        }

    } else {
        while (srcExtent.x < mipTestExtent.x || srcExtent.y < mipTestExtent.y) {
            mipTestExtent.x <<= 1ui32;
            mipTestExtent.y <<= 1ui32;
            --mipDiff;
        }
    }

    assert(static_cast<s32>(options_.mip) + mipDiff >= 0);

    /**
    *
    */
    const auto srcLayer { dst_->baseLayer() - options_.layer };
    const auto srcMip { options_.mip + mipDiff };

    if (srcMip > header.levelCount || header.ss) {
        #ifdef _DEBUG
        __debugbreak();
        #endif
        return;
    }

    #pragma region GLI Source Data

    //_STD fseek(file, 0, SEEK_SET);

    auto internalLevel { indexedLevels[srcMip] };
    auto blockSize { formatDataSize(api::vkTranslateFormat(format)) };

    _STD vector<char> data {};

    /**
    *
    */

    math::uivec3 mipExtent {
        header.pixelWidth >> srcMip, header.pixelHeight >> srcMip, header.pixelDepth >> srcMip
    };
    mipExtent = math::compMax<u32>(mipExtent, math::uivec3 { 1ui32 });

    /**
    *
    */
    const bool useFastCopy { options_.extent.zero() || options_.extent == mipExtent };
    if (useFastCopy) {

        data.resize(internalLevel.byteSize);

        /*
        _STD fseek(file, internalLevel.byteOff, SEEK_SET);
        _STD fread(data.data(), 1, internalLevel.byteSize, file);
         */

        streamsize bytes {};
        src_->get(internalLevel.byteOff, internalLevel.byteSize, data.data(), bytes);

    } else {

        // :: level
        // :: level :: layer
        // :: level :: layer :: face
        // :: level :: layer :: face :: z
        // :: level :: layer :: face :: z :: y
        // :: level :: layer :: face :: z :: y :: x

        const auto levelOffset { internalLevel.byteOff };

        const auto layerCount { header.layerCount ? header.layerCount : 1ui32 };
        const auto faceCount { header.faceCount ? header.faceCount : 1ui32 };

        //
        data.resize(reqExtent.x * reqExtent.y * reqExtent.z * blockSize);

        //
        const auto layerOffset { options_.layer * faceCount * mipExtent.z * mipExtent.y * mipExtent.x };

        const auto minExt { reqOffset };
        const auto maxExt { reqOffset + reqExtent };

        const auto outerOffset { levelOffset + layerOffset };
        auto bfd { 0ui64 };

        // for (u32 face { 0ui32 }; face < faceCount; ++face) {
        for (u32 z { minExt.z }; z < maxExt.z; ++z) {
            for (u32 y { minExt.y }; y < maxExt.y; ++y) {

                // the file is linearized at the x-dimension, so we can read the whole contiguous sequence
                const auto count { maxExt.x - minExt.x };

                u64 fwo { 0ui64 };
                fwo += minExt.x;
                fwo += y * mipExtent.x;
                fwo += z * mipExtent.y * mipExtent.x;

                fwo *= blockSize;

                /*
                _STD fseek(file, outerOffset + fwo, SEEK_SET);
                _STD fread(data.data() + bfd, 1, count * blockSize, file);
                 */

                streamsize bytes {};
                src_->get(outerOffset + fwo, count * blockSize, data.data(), bytes);

                bfd += (count * blockSize);
            }
        }
        // }

    }

    //const auto mipLevelDataSize { glitex.size(srcMip) };
    //auto* const dataBasePtr { glitex.data(srcLayer, 0ui32, srcMip) };

    const auto mipLevelDataSize { data.size() };
    auto* const dataBasePtr { data.data() };

    #pragma endregion

    /**
    * Staging Buffer
    */
    Buffer stage {};
    stage.size = mipLevelDataSize;
    stage.usageFlags = vk::BufferUsageFlagBits::eTransferSrc;

    vk::BufferCreateInfo bci {
        vk::BufferCreateFlags(),
        mipLevelDataSize,
        vk::BufferUsageFlagBits::eTransferSrc,
        vk::SharingMode::eExclusive,
        0,
        nullptr
    };

    stage.buffer = device_->vkDevice().createBuffer(bci);
    stage.device = device_->vkDevice();

    const auto allocResult {
        memory::allocate(
            device_->allocator(),
            device_,
            MemoryProperty::eHostVisible,
            stage
        )
    };
    assert(stage.buffer);
    assert(stage.memory);

    /**
    *
    */
    stage.bind();
    stage.mapAligned();
    assert(stage.memory->mapping);

    /**
    * Copy Data
    */
    stage.write(dataBasePtr, mipLevelDataSize);
    stage.flushAligned();

    /**
    *
    */
    stage.unmap();

    /**
    * Fetch Region per Layer
    */
    Vector<vk::BufferImageCopy> regions {};

    /**
    * Copy Buffer Image (2D / 2D Array)
    */
    regions.push_back(
        vk::BufferImageCopy {
            0ui64,
            0ui32,
            0ui32,
            vk::ImageSubresourceLayers {
                vk::ImageAspectFlagBits::eColor,
                options_.mip,
                options_.layer,
                1ui32
            },
            vk::Offset3D(
                reqOffset.x,
                reqOffset.y,
                reqOffset.z
            ),
            vk::Extent3D(
                reqExtent.x,
                reqExtent.y,
                reqExtent.z
            )
        }
    );

    /**
    * Copy Data to Image
    */
    auto pool = device_->transferQueue()->pool();
    pool->lck().acquire();
    CommandBuffer cmd = pool->make();
    cmd.begin();

    vk::ImageMemoryBarrier simb {
        vk::AccessFlags(),
        vk::AccessFlags(),
        vk::ImageLayout::eUndefined,
        vk::ImageLayout::eTransferDstOptimal,
        VK_QUEUE_FAMILY_IGNORED,
        VK_QUEUE_FAMILY_IGNORED,
        dst_->owner()->vkImage(),
        vk::ImageSubresourceRange {
            vk::ImageAspectFlagBits::eColor,
            options_.mip,
            1ui32,
            options_.layer,
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
        &simb
    );

    cmd.vkCommandBuffer().copyBufferToImage(
        stage.buffer,
        dst_->owner()->vkImage(),
        vk::ImageLayout::eTransferDstOptimal,
        static_cast<u32>(regions.size()),
        regions.data()
    );

    /**
    * Restore Layout
    */
    vk::ImageMemoryBarrier eimb {
        vk::AccessFlags(),
        vk::AccessFlags(),
        vk::ImageLayout::eTransferDstOptimal,
        vk::ImageLayout::eShaderReadOnlyOptimal,
        VK_QUEUE_FAMILY_IGNORED,
        VK_QUEUE_FAMILY_IGNORED,
        dst_->owner()->vkImage(),
        vk::ImageSubresourceRange {
            vk::ImageAspectFlagBits::eColor,
            options_.mip,
            1ui32,
            options_.layer,
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
        &eimb
    );

    cmd.end();
    cmd.submitWait();
    cmd.release();

    pool->lck().release();

    /**
    * Cleanup
    */
    stage.destroy();
    //glitex.clear();
}

void transformer::unloadPartialTmp(
    const non_owning_rptr<const engine::assets::Texture> asset_,
    cref<smr<engine::resource::Source>> src_,
    const non_owning_rptr<VirtualTextureView> dst_,
    cref<sptr<Device>> device_,
    const TextureStreamOptions options_
) {

    /* (Optimization I :: Binary Search Begin) */
    const auto flbe {
        _STD ranges::lower_bound(
            dst_->pages(),
            options_.mip,
            _STD ranges::less {},
            [](non_owning_rptr<VirtualTexturePage> entry_) {
                return entry_->mipLevel();
            }
        )
    };
    /**/

    /* (Optimization II :: Early-Exit Iterator Loop) */
    bool changedMemory { false };
    for (
        auto iter { dst_->pages().begin() + _STD distance(dst_->pages().begin(), flbe) };
        iter != dst_->pages().end() && (*iter)->mipLevel() == options_.mip;
        ++iter
    ) {

        auto* page { *iter };

        if (page->layer() != options_.layer) {
            continue;
        }

        bool effected { false };
        if (page->flags() & VirtualTexturePageFlag::eOpaqueTail) {
            // TODO: effected = page->mipLevel() <= options_->mip;

        } else {

            if (page->mipLevel() != options_.mip) {
                continue;
            }

            const auto minPage { page->offset() };
            const auto maxPage { page->offset() + page->extent() };

            const auto minDst { options_.offset };
            const auto maxDst { options_.offset + options_.extent };

            const auto pX { minPage.x >= maxDst.x || maxPage.x <= minDst.x };
            const auto pY { minPage.y >= maxDst.y || maxPage.y <= minDst.y };
            const auto pZ { minPage.z >= maxDst.z || maxPage.z <= minDst.z };

            effected = !(pX || pY || pZ);
        }

        if (effected && page->memory()->state() == VirtualMemoryPageState::eLoaded) {
            dst_->owner()->unload(page);
            changedMemory = true;
        }
    }

    /**
    *
    */
    if (changedMemory) {
        const auto* tex { dst_->owner() };
        const_cast<VirtualTexture*>(tex)->updateBindingData();
        #pragma warning(push)
        #pragma warning(disable: 4996)
        const_cast<VirtualTexture*>(tex)->enqueueBindingSync(device_->transferQueue());
        #pragma warning(pop)
    }
}
