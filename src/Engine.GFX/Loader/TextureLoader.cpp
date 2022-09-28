#include "TextureLoader.hpp"

#include <fstream>
#include <Engine.Common/Concurrent/Promise.hpp>

#include "../API/VkTranslate.hpp"
#include "../Buffer/Buffer.hpp"
#include "../Cache/GlobalCacheCtrl.hpp"
#include "../Cache/GlobalResourceCache.hpp"
#include "../Command/CommandBuffer.hpp"
#include "../Memory/VkAllocator.hpp"
#include "../Texture/TextureFactory.hpp"
#include "../Texture/VirtualTexturePage.hpp"

using namespace ember::engine::gfx::loader;
using namespace ember::engine::gfx;
using namespace ember;

Texture load_impl(const Url& url_,
    cref<sptr<Device>> device_) {

    //auto url { url_.encode() };
    auto url = url_.path();
    gli::texture glitex = gli::load(url.data());

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

    switch (lvlZeroForm) {
        /**
         * R Formats (2D Images)
         *
         * Used for alpha, roughness, displacement
         */
        case gli::FORMAT_R16_SFLOAT_PACK16: {
            format = vk::Format::eR16Sfloat;
            aspect = vk::ImageAspectFlagBits::eColor;
            break;
        }
        case gli::FORMAT_R32_SFLOAT_PACK32: {
            format = vk::Format::eR32Sfloat;
            aspect = vk::ImageAspectFlagBits::eColor;
            break;
        }

        /**
         * D Formats (2D Images)
         *
         * Used for depth images or possible for alpha blending
         */
        case gli::FORMAT_D16_UNORM_PACK16: {
            format = vk::Format::eD16Unorm;
            aspect = vk::ImageAspectFlagBits::eDepth;
            break;
        }
        case gli::FORMAT_D32_SFLOAT_PACK32: {
            format = vk::Format::eD32Sfloat;
            aspect = vk::ImageAspectFlagBits::eDepth;
            break;
        }

        /**
         * DS Formats (2D Images)
         *
         * Used for depth stencil images
         */
        case gli::FORMAT_D16_UNORM_S8_UINT_PACK32: {
            format = vk::Format::eD16UnormS8Uint;
            aspect = vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil;
            break;
        }
        case gli::FORMAT_D32_SFLOAT_S8_UINT_PACK64: {
            format = vk::Format::eD32SfloatS8Uint;
            aspect = vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil;
            break;
        }

        /**
         * RGB Formats (2D Images)
         *
         * Used for color textures like albedo, sample maps or general image
         */
        case gli::FORMAT_RGB8_UNORM_PACK8: {
            format = vk::Format::eR8G8B8Unorm;
            aspect = vk::ImageAspectFlagBits::eColor;
            break;
        }
        case gli::FORMAT_RGB8_SNORM_PACK8: {
            format = vk::Format::eR8G8B8Snorm;
            aspect = vk::ImageAspectFlagBits::eColor;
            break;
        }
        case gli::FORMAT_RGB16_UNORM_PACK16: {
            format = vk::Format::eR16G16B16A16Unorm;
            aspect = vk::ImageAspectFlagBits::eColor;
            break;
        }
        case gli::FORMAT_RGB32_UINT_PACK32: {
            format = vk::Format::eR32G32B32Uint;
            aspect = vk::ImageAspectFlagBits::eColor;
            break;
        }
        case gli::FORMAT_RGB32_SFLOAT_PACK32: {
            format = vk::Format::eR32G32B32Sfloat;
            aspect = vk::ImageAspectFlagBits::eColor;
            break;
        }

        /**
         * RGBA Formats (2D Images)
         *
         * Used for color textures with alpha like albedo + blending or sample maps
         */
        case gli::FORMAT_RGBA8_UNORM_PACK8: {
            format = vk::Format::eR8G8B8A8Unorm;
            aspect = vk::ImageAspectFlagBits::eColor;
            break;
        }
        case gli::FORMAT_RGBA8_SRGB_PACK8: {
            format = vk::Format::eR8G8B8A8Srgb;
            aspect = vk::ImageAspectFlagBits::eColor;
            break;
        }
        case gli::FORMAT_RGBA8_SNORM_PACK8: {
            format = vk::Format::eR8G8B8A8Snorm;
            aspect = vk::ImageAspectFlagBits::eColor;
            break;
        }
        case gli::FORMAT_RGBA16_UNORM_PACK16: {
            format = vk::Format::eR16G16B16A16Unorm;
            aspect = vk::ImageAspectFlagBits::eColor;
            break;
        }
        case gli::FORMAT_RGBA32_UINT_PACK32: {
            format = vk::Format::eR32G32B32A32Uint;
            aspect = vk::ImageAspectFlagBits::eColor;
            break;
        }
        case gli::FORMAT_RGBA32_SFLOAT_PACK32: {
            format = vk::Format::eR32G32B32A32Sfloat;
            aspect = vk::ImageAspectFlagBits::eColor;
            break;
        }

        /**
         * RGB(A) Formats (Compressed Cube Images)
         */
        case gli::FORMAT_RGBA_ASTC_8X8_UNORM_BLOCK16: {
            format = vk::Format::eAstc8x8UnormBlock;
            aspect = vk::ImageAspectFlagBits::eColor;
            break;
        }
        case gli::FORMAT_RGB_ETC2_UNORM_BLOCK8: {
            format = vk::Format::eEtc2R8G8B8UnormBlock;
            aspect = vk::ImageAspectFlagBits::eColor;
            break;
        }
        case gli::FORMAT_RGBA_DXT5_UNORM_BLOCK16: {
            // TODO: Temporary
            format = vk::Format::eBc2UnormBlock;
            aspect = vk::ImageAspectFlagBits::eColor;
            break;
        }
        default: {
            throw _STD exception("Unresolved texture format.");
        }
    }

    const auto faceCount = glitex.faces();
    if (faceCount == 6) {
        create = vk::ImageCreateFlagBits::eCubeCompatible;
    }

    /**
     * Create Texture
     */
    auto tf = TextureFactory::get();

    const TextureBuildPayload payload {
        math::uivec3 {
            static_cast<u32>(extent.x),
            static_cast<u32>(extent.y),
            static_cast<u32>(extent.z)
        },
        TextureFormat { api::vkTranslateFormat(format) },
        static_cast<u32>(glitex.levels()),
        create & vk::ImageCreateFlagBits::eCubeCompatible ?
            TextureType::eCube :
            TextureType::e2d,
        aspect,
        usage,
        vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
        vk::SharingMode::eExclusive
    };

    Texture result = tf->build(payload);

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
    Vector<vk::BufferImageCopy> regions {};
    uint32_t offset = 0;

    if (create & vk::ImageCreateFlagBits::eCubeCompatible) {

        const gli::texture_cube ct { glitex };

        for (uint32_t face = 0; face < glitex.faces(); face++) {
            for (uint32_t level = 0; level < result.mipLevels(); ++level) {

                const auto se = ct[face][level].extent();

                vk::BufferImageCopy copy {
                    offset,
                    0,
                    0,
                    {
                        aspect,
                        level,
                        face,
                        result.depth()
                    },
                    vk::Offset3D(),
                    vk::Extent3D {
                        static_cast<uint32_t>(se.x),
                        static_cast<uint32_t>(se.y),
                        result.depth()
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
        for (uint32_t level = 0; level < result.mipLevels(); ++level) {
            vk::BufferImageCopy copy {
                offset,
                0,
                0,
                {
                    aspect,
                    level,
                    0,
                    result.type() == TextureType::e2dArray ? result.depth() : 1
                },
                vk::Offset3D(),
                vk::Extent3D {
                    result.width() / (0x1 << level),
                    result.height() / (0x1 << level),
                    result.type() == TextureType::e2dArray ? 1 : result.depth()
                }
            };

            regions.push_back(copy);
            offset += static_cast<uint32_t>(glitex.size(level));
        }
    }

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
        result.buffer().image(),
        {
            aspect,
            0,
            result.mipLevels(),
            0,
            result.layer()
        }
    };

    cmd.vkCommandBuffer().pipelineBarrier(
        vk::PipelineStageFlagBits::eAllCommands,
        vk::PipelineStageFlagBits::eAllCommands,
        vk::DependencyFlags(),
        0, nullptr,
        0, nullptr,
        1, &simb
    );

    result.buffer()._vkLayout = vk::ImageLayout::eTransferDstOptimal;

    for (auto& entry : regions) {
        cmd.copyBufferToImage(
            stage, result.buffer(), entry
        );
    }

    /**
     * Restore Layout
     */
    vk::ImageMemoryBarrier eimb {
        vk::AccessFlags(),
        vk::AccessFlags(),
        vk::ImageLayout::eTransferDstOptimal,
        vk::ImageLayout::eTransferSrcOptimal,
        VK_QUEUE_FAMILY_IGNORED,
        VK_QUEUE_FAMILY_IGNORED,
        result.buffer().image(),
        {
            aspect,
            0,
            result.mipLevels(),
            0,
            result.layer()
        }
    };

    cmd.vkCommandBuffer().pipelineBarrier(
        vk::PipelineStageFlagBits::eAllCommands,
        vk::PipelineStageFlagBits::eAllCommands,
        vk::DependencyFlags(),
        0, nullptr,
        0, nullptr,
        1, &eimb
    );

    cmd.end();
    cmd.submitWait();
    cmd.release();

    pool->lck().release();

    /**
     * Cleanup
     */
    stage.destroy();
    glitex.clear();

    result.buffer()._vkLayout = vk::ImageLayout::eTransferSrcOptimal;
    return result;
}

TextureLoader::TextureLoader(const ptr<cache::GlobalCacheCtrl> cache_) :
    _cacheCtrl(cache_) {}

ember::concurrent::promise<Texture> TextureLoader::load(
    const Url& url_) const {
    concurrent::promise<Texture> p {
        [_device = _cacheCtrl->cache()->device(), url_ = url_]() {
            return load_impl(url_, _device);
        }
    };

    return p;
}

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

unsigned char ktx20Identifier[] = {
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

uptr<VirtualTextureView> TextureLoader::loadTo(
    const Url& url_,
    mref<uptr<VirtualTextureView>> dst_,
    const TextureLoaderFlags flags_
) const {

    /**/
    assert(url_.scheme() == "file"sv);
    assert(url_.hasPath());

    auto dst { _STD move(dst_) };
    assert(dst->type() == TextureType::e2d || dst->type() == TextureType::e2dArray);
    /**/

    auto url = url_.path();
    gli::texture glitex {};

    /**/
    math::ivec3 extent {};

    vk::Format format;
    vk::ImageAspectFlags aspect;

    u32 effectedMipLevels { 0ui32 };
    u32 layerCount { 1ui32 };
    /**/

    if (flags_ & TextureLoaderFlagBits::eLazyDataLoading) {
        /**
         * Just load header and meta data
         *
         * @see gli::load(...) load.inl#L9
         */
        FILE* file { gli::detail::open_file(url.data(), "rb") };
        assert(file);

        auto beg { _STD ftell(file) };
        _STD fseek(file, 0, SEEK_END);
        auto end { _STD ftell(file) };
        _STD fseek(file, 0, SEEK_SET);

        auto size { end - beg };
        constexpr auto header_min_size { sizeof(gli::detail::ktx_header10) };
        assert(size > header_min_size);

        constexpr auto mem_page_size { 4096ui64 };
        const auto chunkSize { mem_page_size < size ? mem_page_size : size };
        _STD vector<char> raw(static_cast<size_t>(chunkSize));

        _STD fread(raw.data(), 1, chunkSize, file);
        _STD fclose(file);

        #if FALSE
        /**
         * Internal gli
         */
        if (memcmp(raw.data(), gli::detail::FOURCC_KTX10, sizeof(gli::detail::FOURCC_KTX10)) != 0) {
        #ifdef _DEBUG
            __debugbreak();
            return _STD move(dst);
        #else
            throw _STD runtime_error("Unsupported texture file (based on KTX 1.0 header)");
        #endif
        }

        auto* cursor { raw.data() + sizeof(gli::detail::FOURCC_KTX10) };
        cref<gli::detail::ktx_header10> header { *reinterpret_cast<ptr<const gli::detail::ktx_header10>>(cursor) };
        #endif

        auto* cursor { raw.data() + 12ui64 };
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
        const_cast<ref<u32>>(effectedMipLevels) = _STD min(header.levelCount, dst->mipLevels());

    } else {
        /**
         * Fully load source file
         */
        glitex = gli::load(url.data());

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

    if (!(flags_ & TextureLoaderFlagBits::eLazyDataLoading)) {

        /**
         * Setup vulkan stage buffer to eager load texture
         */
        vk::BufferCreateInfo bci {
            vk::BufferCreateFlags(),
            glitex.size(),
            vk::BufferUsageFlagBits::eTransferSrc,
            vk::SharingMode::eExclusive,
            0,
            nullptr
        };

        stage.buffer = _cacheCtrl->cache()->device()->vkDevice().createBuffer(bci);
        stage.device = _cacheCtrl->cache()->device()->vkDevice();

        const auto allocResult {
            memory::allocate(
                _cacheCtrl->cache()->device()->allocator(),
                _cacheCtrl->cache()->device(),
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
        uint32_t srcOff = 0;

        /**
         * Copy Buffer Image (2D / 2D Array)
         */
        for (uint32_t level = 0; level < effectedMipLevels; ++level) {

            const auto srcExt { glitex.extent(level) };

            vk::BufferImageCopy copy {
                srcOff,
                0,
                0,
                {
                    aspect,
                    level,
                    dst->baseLayer(),
                    layerCount
                },
                vk::Offset3D(),
                vk::Extent3D {
                    static_cast<u32>(srcExt.x),
                    static_cast<u32>(srcExt.y),
                    static_cast<u32>(srcExt.z)
                }
            };

            regions.push_back(copy);
            srcOff += static_cast<uint32_t>(glitex.size(level));
        }

        // Warning: Temporary
        for (auto* page : dst->_pages) {
            if (page->memory()->state() != VirtualMemoryPageState::eLoaded) {
                page->load();
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
    const_cast<VirtualTexture*>(tex)->enqueueBindingSync(_cacheCtrl->cache()->device()->graphicsQueue());
    #pragma warning(pop)

    /**
     * Transform and data transfer
     */
    auto pool = _cacheCtrl->cache()->device()->transferQueue()->pool();
    pool->lck().acquire();
    CommandBuffer cmd = pool->make();
    cmd.begin();

    /**/
    vk::ImageMemoryBarrier preBarrier {};
    vk::ImageMemoryBarrier postBarrier {};
    /**/

    if (!(flags_ & TextureLoaderFlagBits::eLazyDataLoading)) {
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
            0, nullptr,
            0, nullptr,
            1, &preBarrier
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
        (flags_ & TextureLoaderFlagBits::eLazyDataLoading) ?
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
        vk::PipelineStageFlagBits::eTransfer,
        vk::DependencyFlags(),
        0, nullptr,
        0, nullptr,
        1, &postBarrier
    );

    cmd.end();
    cmd.submitWait();
    cmd.release();

    pool->lck().release();

    /**
     * Cleanup
     */
    if (!(flags_ & TextureLoaderFlagBits::eLazyDataLoading)) {
        stage.destroy();
        glitex.clear();
    }

    return _STD move(dst);
}

void TextureLoader::partialLoadTo(
    const Url& url_,
    const ptr<const ember::engine::res::StreamLoaderOptions<ember::engine::assets::Texture>> options_,
    const ptr<VirtualTextureView> dst_
) const {

    /**
     *
     */
    auto url = url_.path();
    // gli::texture glitex = gli::load(url.data());

    #pragma region GLI Header

    FILE* file { gli::detail::open_file(url.data(), "rb") };
    assert(file);

    auto beg { _STD ftell(file) };
    _STD fseek(file, 0, SEEK_END);
    auto end { _STD ftell(file) };
    _STD fseek(file, 0, SEEK_SET);

    auto size { end - beg };
    constexpr auto header_min_size { sizeof(InternalKtxHeader) };
    assert(size > header_min_size);

    constexpr auto mem_page_size { 4096ui64 };
    const auto chunkSize { mem_page_size < size ? mem_page_size : size };
    _STD vector<char> raw(static_cast<size_t>(chunkSize));

    _STD fread(raw.data(), 1, chunkSize, file);
    _STD fclose(file);

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

    assert(static_cast<s32>(options_->mip) + mipDiff >= 0);

    /**
     *
     */
    const auto srcLayer { dst_->baseLayer() - options_->layer };
    const auto srcMip { options_->mip + mipDiff };

    if (srcMip > header.levelCount || header.ss) {
        #ifdef _DEBUG
        __debugbreak();
        #endif
        return;
    }

    #pragma region GLI Source Data

    file = gli::detail::open_file(url.data(), "rb");
    assert(file);

    //_STD fseek(file, 0, SEEK_SET);

    auto internalLevel { indexedLevels[srcMip] };
    auto blockSize { formatDataSize(api::vkTranslateFormat(format)) };
    _STD vector<char> data {};

    /**
     *
     */

    math::uivec3 mipExtent { header.pixelWidth >> srcMip, header.pixelHeight >> srcMip, header.pixelDepth >> srcMip };

    mipExtent.x = mipExtent.x ? mipExtent.x : 1ui32;
    mipExtent.y = mipExtent.y ? mipExtent.y : 1ui32;
    mipExtent.z = mipExtent.z ? mipExtent.z : 1ui32;

    /**
     *
     */
    const bool useFastCopy { options_->extent.zero() || options_->extent == mipExtent };
    if (useFastCopy) {

        data.resize(internalLevel.byteSize);

        _STD fseek(file, internalLevel.byteOff, SEEK_SET);
        _STD fread(data.data(), 1, internalLevel.byteSize, file);

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
        data.resize(options_->extent.x * options_->extent.y * options_->extent.z * blockSize);

        //
        const auto layerOffset { options_->layer * faceCount * mipExtent.z * mipExtent.y * mipExtent.x };

        const auto minExt { options_->offset };
        const auto maxExt { options_->offset + options_->extent };

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

                _STD fseek(file, outerOffset + fwo, SEEK_SET);
                _STD fread(data.data() + bfd, 1, count * blockSize, file);

                bfd += (count * blockSize);
            }
        }
        // }

    }

    _STD fclose(file);

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

    stage.buffer = _cacheCtrl->cache()->device()->vkDevice().createBuffer(bci);
    stage.device = _cacheCtrl->cache()->device()->vkDevice();

    const auto allocResult {
        memory::allocate(
            _cacheCtrl->cache()->device()->allocator(),
            _cacheCtrl->cache()->device(),
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
    regions.push_back(vk::BufferImageCopy {
        0ui64,
        0ui32,
        0ui32,
        vk::ImageSubresourceLayers {
            vk::ImageAspectFlagBits::eColor,
            options_->mip,
            options_->layer,
            1ui32
        },
        vk::Offset3D(
            options_->offset.x,
            options_->offset.y,
            options_->offset.z
        ),
        vk::Extent3D(
            options_->extent.x,
            options_->extent.y,
            options_->extent.z
        )
    });

    // Warning: Temporary
    for (auto* page : dst_->_pages) {

        if (page->layer() != options_->layer) {
            continue;
        }

        bool effected { false };
        if (page->flags() & VirtualTexturePageFlag::eOpaqueTail) {
            effected = page->mipLevel() <= options_->mip;

        } else {

            if (page->mipLevel() != options_->mip) {
                continue;
            }

            const auto minPage { page->offset() };
            const auto maxPage { page->offset() + page->extent() };

            const auto minDst { options_->offset };
            const auto maxDst { options_->offset + options_->extent };

            const auto pX { minPage.x >= maxDst.x || maxPage.x <= minDst.x };
            const auto pY { minPage.y >= maxDst.y || maxPage.y <= minDst.y };
            const auto pZ { minPage.z >= maxDst.z || maxPage.z <= minDst.z };

            effected = !(pX || pY || pZ);
        }

        if (effected && page->memory()->state() != VirtualMemoryPageState::eLoaded) {
            page->load();
        }
    }

    const auto* tex { dst_->owner() };
    const_cast<VirtualTexture*>(tex)->updateBindingData();
    #pragma warning(push)
    #pragma warning(disable: 4996)
    const_cast<VirtualTexture*>(tex)->enqueueBindingSync(_cacheCtrl->cache()->device()->graphicsQueue());
    #pragma warning(pop)

    /**
     * Copy Data to Image
     */
    auto pool = _cacheCtrl->cache()->device()->transferQueue()->pool();
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
            options_->mip,
            1ui32,
            options_->layer,
            1ui32
        }
    };

    cmd.vkCommandBuffer().pipelineBarrier(
        vk::PipelineStageFlagBits::eTransfer,
        vk::PipelineStageFlagBits::eTransfer,
        vk::DependencyFlags(),
        0, nullptr,
        0, nullptr,
        1, &simb
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
            options_->mip,
            1ui32,
            options_->layer,
            1ui32
        }
    };

    cmd.vkCommandBuffer().pipelineBarrier(
        vk::PipelineStageFlagBits::eTransfer,
        vk::PipelineStageFlagBits::eTransfer,
        vk::DependencyFlags(),
        0, nullptr,
        0, nullptr,
        1, &eimb
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

void TextureLoader::partialUnload(
    const ptr<const ember::engine::res::StreamLoaderOptions<ember::engine::assets::Texture>> options_,
    const ptr<VirtualTextureView> dst_
) const {

    // Warning: Temporary
    for (auto* page : dst_->_pages) {

        if (page->layer() != options_->layer) {
            continue;
        }

        bool effected { false };
        if (page->flags() & VirtualTexturePageFlag::eOpaqueTail) {
            effected = page->mipLevel() <= options_->mip;

        } else {

            if (page->mipLevel() != options_->mip) {
                continue;
            }

            const auto minPage { page->offset() };
            const auto maxPage { page->offset() + page->extent() };
            const auto minDst { options_->offset };
            const auto maxDst { options_->offset + options_->extent };

            const auto pX { minPage.x > maxDst.x || maxPage.x < minDst.x };
            const auto pY { minPage.y > maxDst.y || maxPage.y < minDst.y };
            const auto pZ { minPage.z > maxDst.z || maxPage.z < minDst.z };

            effected = !(pX || pY || pZ);
        }

        if (effected && page->memory()->state() == VirtualMemoryPageState::eLoaded) {
            page->unload();
        }
    }

    const auto* tex { dst_->owner() };
    const_cast<VirtualTexture*>(tex)->updateBindingData();
    #pragma warning(push)
    #pragma warning(disable: 4996)
    const_cast<VirtualTexture*>(tex)->enqueueBindingSync(_cacheCtrl->cache()->device()->graphicsQueue());
    #pragma warning(pop)
}

Texture TextureLoader::__tmp__load(const Url& url_) {
    return load_impl(url_, _cacheCtrl->cache()->device());
}

#if FALSE

__Texture2d TextureLoader::load2dToBuffer(const gli::texture& tex_, const vk::Format& format_, const __Queue& queue_,
	const vk::ImageUsageFlags& imageFlags_, const vk::ImageLayout& imageLayout_, const bool& staging_) const {
	gli::texture2d tex = gli::texture2d(tex_);

	__Texture2d texture = __internal::types::shared<Texture2d>(_device, tex[0].extent().x, tex[0].extent().y,
		tex.levels());
	texture->meta().usage(imageFlags_);
	texture->meta().format(format_);

	const vk::FormatProperties formatProps = _device->physDev.getFormatProperties(format_);

	// Use a separate command buffer for texture loading
	const vk::CommandBuffer copyCmd = _device->createCommandBuffer(vk::CommandBufferLevel::ePrimary,
		queue_->familyIndex(), true);
	copyCmd.begin(vk::CommandBufferBeginInfo());

	if (staging_) {
		// Comment: vk::MemoryPropertyFlagBits::eDeviceLocal will cause a unaccessible eHostVisible and eHostCoherent
// Comment: !!! Do not use DeviceLocal and HostVisible/Coherent at once
//texture->memoryFlags(texture->memoryFlags()/* | vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent */);

		/**
		 *	Create new texture
		 */
		texture->init(*_initializer);
#ifdef _DEBUG
		DEBUG_NMSG("GUT", "Created target 2d texture while loading.")
#endif

		/**
		 *	Copy data to texture
		 */
		Buffer staging = _device->createBuffer(vk::BufferUsageFlagBits::eTransferSrc,
			vk::MemoryPropertyFlagBits::eHostVisible | vk::
			MemoryPropertyFlagBits::eHostCoherent, tex.size());
		staging.bind();

#ifdef _DEBUG
		DEBUG_NMSG("GUT", "Created staging buffer for 2d texture loading.")
#endif

		staging.map();
		staging.copyTo(tex.data(), tex.size());
		staging.unmap();

		// Setup buffer copy regions for each mip level
		std::vector<vk::BufferImageCopy> copyRegions = std::vector<vk::BufferImageCopy>(0);
		uint32_t offset = 0;

		for (uint32_t i = 0; i < texture->meta().mipLevels(); i++) {
			vk::BufferImageCopy region = vk::BufferImageCopy();
			region.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
			region.imageSubresource.mipLevel = i;
			region.imageSubresource.baseArrayLayer = 0;
			region.imageSubresource.layerCount = 1;
			region.imageExtent.width = static_cast<uint32_t>(tex[i].extent().x);
			region.imageExtent.height = static_cast<uint32_t>(tex[i].extent().y);
			region.imageExtent.depth = 1;
			region.bufferOffset = offset;

			copyRegions.push_back(region);

			offset += static_cast<uint32_t>(tex[i].size());
		}

		if (!(texture->meta().usage() & vk::ImageUsageFlagBits::eTransferDst))
			texture->meta().usage(texture->meta().usage() | vk::ImageUsageFlagBits::eTransferDst);

		texture->toImageLayout(vk::ImageLayout::eTransferDstOptimal, copyCmd);
		copyCmd.copyBufferToImage(staging.buffer, texture->image().image, vk::ImageLayout::eTransferDstOptimal,
			static_cast<uint32_t>(copyRegions.size()), copyRegions.data());
		texture->toImageLayout(imageLayout_, copyCmd);

		_device->flushCmdBuffer(copyCmd, queue_, true);
		staging.destroy();
	} else {
		// Bug: Broke - Image Raw Memory is not the same as buffer raw memory and raw data of image in file
// TODO: lookup whether raw image memory is fillable by ktx file format or whether stage algorithm is required
		/**
		 *	Create new texture
		 */
		assert(formatProps.linearTilingFeatures & vk::FormatFeatureFlagBits::eSampledImage);
		texture->image().memoryFlags = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::
			eHostCoherent;

		texture->init(*_initializer);
#ifdef _DEBUG
		DEBUG_NMSG("GUT", "Created visible target 2d texture while loading.")
#endif

		/**
		 *	Copy data to texture
		 */
		vk::ImageSubresource subRes = vk::ImageSubresource();
		subRes.aspectMask = vk::ImageAspectFlagBits::eColor;
		subRes.mipLevel = 0;
		subRes.arrayLayer = 0;

		const vk::MemoryRequirements memReq = _device->logDev.getImageMemoryRequirements(texture->image().image);
		vk::SubresourceLayout subResLayout = _device->logDev.getImageSubresourceLayout(texture->image().image, subRes);
		void* data;

		vk::CommandBuffer cmd = _device->createCommandBuffer(vk::CommandBufferLevel::ePrimary, queue_->familyIndex(),
			true);
		cmd.begin(vk::CommandBufferBeginInfo());
		texture->toImageLayout(vk::ImageLayout::eTransferDstOptimal, cmd);
		_device->flushCmdBuffer(cmd, _device->transferQueue, true);

		texture->image().map();
		texture->image().copyTo(tex[subRes.mipLevel].data(), tex[subRes.mipLevel].size());
		texture->image().unmap();

		texture->toImageLayout(imageLayout_, copyCmd);

		_device->flushCmdBuffer(copyCmd, queue_, true);
	}

	texture->update();
	return texture;
}

__TextureCubemap TextureLoader::loadCubeToBuffer(const gli::texture& tex_, const vk::Format& format_,
	const __Queue& queue_, const vk::ImageUsageFlags& imageFlags_, const vk::ImageLayout& imageLayout_,
	const bool& staging_) const {
	gli::texture_cube tex = gli::texture_cube(tex_);

	__TextureCubemap texture = __internal::types::shared<TextureCubemap>(_device, tex[0].extent().x, tex[0].extent().y,
		tex[0].levels());
	texture->meta().flags(vk::ImageCreateFlagBits::eCubeCompatible);
	texture->meta().format(format_);
	texture->meta().usage(imageFlags_);

	texture->samplerMeta().addressMode(
		vk::SamplerAddressMode::eClampToEdge,
		vk::SamplerAddressMode::eClampToEdge,
		vk::SamplerAddressMode::eClampToEdge
	);

	vk::CommandBuffer cmd = _device->createCommandBuffer(vk::CommandBufferLevel::ePrimary, queue_->familyIndex(), true);
	cmd.begin(vk::CommandBufferBeginInfo());

	if (!(texture->meta().usage() & vk::ImageUsageFlagBits::eTransferDst))
		texture->meta().usage(texture->meta().usage() | vk::ImageUsageFlagBits::eTransferDst);

	/**
	 *	Create new texture
	 */
	texture->init(*_initializer);

	/**
	 *	Copy data to texture
	 */
	Buffer staging = _device->createBuffer(
		vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eTransferSrc,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, tex.size(), tex.data());
	staging.bind();

	std::vector<vk::BufferImageCopy> regions;
	uint32_t offset = 0;

	for (int32_t face = 0; face < texture->meta().depth(); face++)
		for (uint32_t level = 0; level < texture->meta().mipLevels(); level++) {
			vk::BufferImageCopy region = vk::BufferImageCopy();
			region.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
			region.imageSubresource.mipLevel = level;
			region.imageSubresource.baseArrayLayer = face;
			region.imageSubresource.layerCount = 1;
			region.imageExtent = vk::Extent3D(
				UINT32_T(tex[face][level].extent().x), UINT32_T(tex[face][level].extent().y), 1);
			region.bufferOffset = offset;

			regions.push_back(region);
			offset += UINT32_T(tex[face][level].size());
		}

	vk::ImageSubresourceRange isr = vk::ImageSubresourceRange();
	isr.aspectMask = vk::ImageAspectFlagBits::eColor;
	isr.baseMipLevel = 0;
	isr.levelCount = texture->meta().mipLevels();
	isr.layerCount = texture->meta().depth();

	texture->toImageLayout(vk::ImageLayout::eTransferDstOptimal, cmd);
	cmd.copyBufferToImage(staging.buffer, texture->image().image, vk::ImageLayout::eTransferDstOptimal,
		UINT32_T(regions.size()), regions.data());
	texture->toImageLayout(imageLayout_, cmd);
	
	_device->flushCmdBuffer(cmd, queue_, true);

	staging.destroy();

	return texture;
}

TextureLoader::future_2d_type TextureLoader::load2dFromFile(const _STD string& file_, const vk::Format& format_, const
	__Queue& queue_, const vk::ImageUsageFlags& flags_, const vk::ImageLayout& layout_, const bool& staging_) const {
	gli::texture2d glit(gli::load(file_));
#ifdef _DEBUG
	DEBUG_NMSG("GUT", "Loading 2d texture from file ('" + file_ + "').");
#endif

	return tasking::async<__Texture2d>([=] {
		return this->load2dToBuffer(glit, format_, queue_, flags_, layout_, staging_);
	});
}

TextureLoader::future_cube_type TextureLoader::loadCubeFromFile(const _STD string& file_, const vk::Format& format_,
	const
	__Queue& queue_, const vk::ImageUsageFlags& flags_, const vk::ImageLayout& layout_, const bool& staging_) const {
	gli::texture_cube glit(gli::load(file_));
#ifdef _DEBUG
	DEBUG_NMSG("GUT", "Loading 2d texture from file ('" + file_ + "').");
#endif
	return tasking::async<__TextureCubemap>([=] {
		return this->loadCubeToBuffer(glit, format_, queue_, flags_, layout_, staging_);
	});
}

#endif
