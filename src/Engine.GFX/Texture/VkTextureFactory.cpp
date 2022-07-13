#include "VkTextureFactory.hpp"

#ifdef _PROFILING
#include <Engine.Common/Profiling/Stopwatch.hpp>
#endif

#include "../API/VkTranslate.hpp"

using namespace ember::engine::gfx;
using namespace ember;

[[nodiscard]] vk::ImageType vkTranslate(const TextureType type_) noexcept {
    switch (type_) {
        case TextureType::e2d:
        case TextureType::e2dArray:
        case TextureType::eCube: {
            return vk::ImageType::e2D;
        }
        case TextureType::e3d: {
            return vk::ImageType::e3D;
        }
        default: {
            return vk::ImageType::e1D;
        }
    }
}

[[nodiscard]] vk::ImageViewType vkTranslateView(TextureType type_) noexcept {
    switch (type_) {
        case TextureType::e2d: {
            return vk::ImageViewType::e2D;
        }
        case TextureType::e2dArray: {
            return vk::ImageViewType::e2DArray;
        }
        case TextureType::eCube: {
            return vk::ImageViewType::eCube;
        }
        case TextureType::e3d: {
            return vk::ImageViewType::e3D;
        }
        default: {
            return vk::ImageViewType::e2D;
        }
    }
}

[[nodiscard]] vk::AccessFlags vkTranslate(const vk::ImageLayout layout_) noexcept {
    switch (layout_) {
        case vk::ImageLayout::eColorAttachmentOptimal: {
            return vk::AccessFlagBits::eColorAttachmentWrite;
        }
        case vk::ImageLayout::eDepthStencilAttachmentOptimal: {
            return vk::AccessFlagBits::eDepthStencilAttachmentWrite;
        }
        case vk::ImageLayout::eTransferDstOptimal: {
            return vk::AccessFlagBits::eTransferWrite;
        }
        case vk::ImageLayout::eTransferSrcOptimal: {
            return vk::AccessFlagBits::eTransferRead;
        }
        case vk::ImageLayout::eShaderReadOnlyOptimal: {
            return vk::AccessFlagBits::eShaderRead;
        }
        default: {
            return vk::AccessFlags();
        }
    }
}

ptr<TextureFactory> VkTextureFactory::make(cref<sptr<Device>> device_) {
    assert(!_instance);

    _instance = new VkTextureFactory(device_);
    return _instance;
}

VkTextureFactory::VkTextureFactory(cref<sptr<Device>> device_) :
    TextureFactory(device_) {}

VkTextureFactory::~VkTextureFactory() noexcept { }

Texture VkTextureFactory::build(const TextureBuildPayload& payload_) const {

    SCOPED_STOPWATCH

    vk::ImageCreateFlags create;
    u32 layers = 1;
    u32 depth = 1;

    if (payload_.type == TextureType::e2d) {
        layers = 1;
        depth = payload_.extent.z;
    } else if (payload_.type == TextureType::e2dArray) {
        layers = payload_.extent.z;
        depth = 1;
    } else if (payload_.type == TextureType::eCube) {
        create |= vk::ImageCreateFlagBits::eCubeCompatible;
        layers = 6;
        depth = payload_.extent.z;
    }

    /**
     * Vk Image
     */
    const vk::ImageCreateInfo ici {
        create,
        vkTranslate(payload_.type),
        api::vkTranslateFormat(payload_.format),
        vk::Extent3D {
            payload_.extent.x,
            payload_.extent.y,
            depth
        },
        payload_.mipLevels,
        layers,
        vk::SampleCountFlagBits::e1,
        vk::ImageTiling::eOptimal,
        payload_.vkUsage,
        payload_.vkSharing,
        {},
        {},
        // Comment: Layout can be changed later, don't make nasty queued command buffer calls while using factory
        vk::ImageLayout::eUndefined
    };

    const vk::Image image = _device->vkDevice().createImage(ici);
    assert(image);

    /**
     * Image Memory
     */
    const vk::MemoryRequirements mr = _device->vkDevice().getImageMemoryRequirements(image);
    const vk::MemoryAllocateInfo mai {
        mr.size,
        get_memory_type(_device->vkPhysicalDevice(), mr.memoryTypeBits, payload_.vkMemoryFlags)
    };

    const vk::DeviceMemory memory = _device->vkDevice().allocateMemory(mai);

    /**
     * Bind Image and Memory
     */
    _device->vkDevice().bindImageMemory(image, memory, 0);

    /**
     * 
     */
    TextureBuffer buffer {};
    buffer.image() = image;
    buffer.memory() = memory;
    buffer.memorySize() = mr.size;
    buffer.memoryAlign() = mr.alignment;

    buffer.sharingMode() = payload_.vkSharing;
    buffer.tiling() = vk::ImageTiling::eOptimal;

    buffer.vkDevice() = _device->vkDevice();
    buffer._vkAspect = payload_.vkAspect;
    buffer._vkCreate = create;
    buffer._vkLayout = vk::ImageLayout::eUndefined;
    buffer._vkUsage = payload_.vkUsage;

    /**
     * 
     */
    Texture texture {};
    texture.type() = payload_.type;
    texture.buffer() = buffer;
    texture.extent() = payload_.extent;
    texture.format() = payload_.format;
    texture.mipLevels() = payload_.mipLevels;

    return texture;
}

Texture& VkTextureFactory::buildView(Texture& texture_) const {

    SCOPED_STOPWATCH

    /**
     * Prepare
     */
    const vk::ComponentMapping cm {
        vk::ComponentSwizzle::eR,
        vk::ComponentSwizzle::eG,
        vk::ComponentSwizzle::eB,
        vk::ComponentSwizzle::eA
    };

    const vk::ImageSubresourceRange isr {
        texture_.buffer()._vkAspect,
        0,
        texture_.mipLevels(),
        0,
        texture_.layer()
    };

    const vk::ImageViewCreateInfo ivci {
        vk::ImageViewCreateFlags(),
        texture_.buffer().image(),
        vkTranslateView(texture_.type()),
        api::vkTranslateFormat(texture_.format()),
        cm,
        isr
    };

    /**
     * Create
     */
    const vk::ImageView view = _device->vkDevice().createImageView(ivci);
    assert(view);

    texture_.vkView() = view;
    return texture_;
}

ptr<VirtualTexture> VkTextureFactory::buildVirtual(const VirtualTextureBuildPayload& payload_) const {

    SCOPED_STOPWATCH

    vk::ImageCreateFlags create { vk::ImageCreateFlagBits::eSparseBinding | vk::ImageCreateFlagBits::eSparseResidency };

    u32 layers { 0ui32 };
    if (payload_.type == TextureType::e2d) {

        #ifdef _DEBUG
        assert(payload_.extent.z == 1ui32);
        assert(payload_.layers <= 1ui32);
        #endif

        layers = 1ui32;

    } else if (payload_.type == TextureType::e2dArray) {

        #ifdef _DEBUG
        assert(payload_.extent.z == 1ui32);
        assert(payload_.layers >= 1ui32);
        #endif

        layers = payload_.layers;

    } else if (payload_.type == TextureType::eCube) {

        #ifdef _DEBUG
        assert(payload_.layers >= 6ui32);
        assert(payload_.extent.x == payload_.extent.y);
        #endif

        create |= vk::ImageCreateFlagBits::eCubeCompatible;
        layers = payload_.layers;
    }

    /**
     * Vk Image
     */
    const vk::ImageCreateInfo ici {
        create,
        vkTranslate(payload_.type),
        api::vkTranslateFormat(payload_.format),
        vk::Extent3D {
            payload_.extent.x,
            payload_.extent.y,
            payload_.extent.z
        },
        payload_.mipLevels.max + 1ui32,
        layers,
        vk::SampleCountFlagBits::e1,
        vk::ImageTiling::eOptimal,
        payload_.vkUsage,
        payload_.vkSharing,
        {},
        {},
        // Comment: Layout can be changed later, don't make nasty queued command buffer calls while using factory
        vk::ImageLayout::eUndefined
    };

    const vk::Image image = _device->vkDevice().createImage(ici);
    assert(image);

    /**
     * Get meta data of required memory
     */
    const vk::MemoryRequirements mr = _device->vkDevice().getImageMemoryRequirements(image);

    /**
     * Get suitable virtual memory instance
     */
    auto memory {
        make_uptr<VirtualMemory>(
            _device->allocator(),
            memory::MemoryLayout {
                mr.alignment,
                api::vkTranslateMemoryProperties(payload_.vkMemoryFlags),
                mr.memoryTypeBits
            },
            mr.size
        )
    };

    /**
     *
     */
    auto* texture {
        make_ptr<VirtualTexture>(
            _STD move(memory),
            layers,
            payload_.extent,
            payload_.format,
            payload_.mipLevels,
            payload_.type,
            image
        )
    };

    // TODO: copy aspects for sparse image binding
    // -> buffer._vkAspect = payload_.vkAspect;

    // TODO: track meta attributes of underlying resources like
    // -> vk::ImageTiling, vk::ImageLayout, vk::ImageUsage

    /**
     * TODO: Remove
     */
    {
        const vk::ComponentMapping cm {
            vk::ComponentSwizzle::eR,
            vk::ComponentSwizzle::eG,
            vk::ComponentSwizzle::eB,
            vk::ComponentSwizzle::eA
        };

        const vk::ImageSubresourceRange isr {
            vk::ImageAspectFlagBits::eColor,
            0,
            static_cast<u32>(payload_.mipLevels.max - payload_.mipLevels.min) + 1ui32,
            0,
            layers
        };

        const vk::ImageViewCreateInfo ivci {
            vk::ImageViewCreateFlags(),
            image,
            vkTranslateView(TextureType::e2dArray),
            api::vkTranslateFormat(payload_.format),
            cm,
            isr
        };

        const vk::ImageView view = _device->vkDevice().createImageView(ivci);
        assert(view);

        texture->_vkImageView = view;
    }

    return texture;
}
