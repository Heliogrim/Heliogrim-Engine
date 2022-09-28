#include "VkTranslate.hpp"

using namespace ember::engine::gfx::api;

vk::PolygonMode ember::engine::gfx::api::vkTranslateMode(RasterPolyMode mode_) {
    switch (mode_) {
        case RasterPolyMode::eFace: {
            return vk::PolygonMode::eFill;
        }
        case RasterPolyMode::eLine: {
            return vk::PolygonMode::eLine;
        }
        default: {
            return vk::PolygonMode::eFill;
        }
    }
}

vk::FrontFace ember::engine::gfx::api::vkTranslateFace(RasterPolyFront front_) {
    switch (front_) {
        case RasterPolyFront::eClockwise: {
            return vk::FrontFace::eClockwise;
        }
        case RasterPolyFront::eCounterClockwise: {
            return vk::FrontFace::eCounterClockwise;
        }
        default: {
            return vk::FrontFace::eClockwise;
        }
    }
}

vk::SampleCountFlagBits ember::engine::gfx::api::vkTranslateSamples(RasterSamples samples_) {
    switch (samples_) {
        case RasterSamples::e1: {
            return vk::SampleCountFlagBits::e1;

        case RasterSamples::e4: {
            return vk::SampleCountFlagBits::e4;
        }
        case RasterSamples::e8: {
            return vk::SampleCountFlagBits::e8;
        }
        case RasterSamples::e16: {
            return vk::SampleCountFlagBits::e16;
        }
        case RasterSamples::e32: {
            return vk::SampleCountFlagBits::e32;
        }
        case RasterSamples::e64: {
            return vk::SampleCountFlagBits::e64;
        }
        default: {
            return vk::SampleCountFlagBits::e1;
        }
        }
    }
}

vk::VertexInputRate ember::engine::gfx::api::vkTranslateInputRate(InputRate rate_) {
    switch (rate_) {
        case InputRate::ePerInstance: {
            return vk::VertexInputRate::eInstance;
        }
        case InputRate::ePerVertex: {
            return vk::VertexInputRate::eVertex;
        }
        default: {
            return vk::VertexInputRate::eVertex;
        }
    }
}

vk::CullModeFlagBits ember::engine::gfx::api::vkTranslateCull(RasterCullFace face_) {
    switch (face_) {
        case RasterCullFace::eNone: {
            return vk::CullModeFlagBits::eNone;
        }
        case RasterCullFace::eBack: {
            return vk::CullModeFlagBits::eBack;
        }
        case RasterCullFace::eFront: {
            return vk::CullModeFlagBits::eFront;
        }
        case RasterCullFace::eBackAndFront: {
            return vk::CullModeFlagBits::eFrontAndBack;
        }
        default: {
            return vk::CullModeFlagBits::eNone;
        }
    }
}

vk::PrimitiveTopology ember::engine::gfx::api::vkTranslateTopology(PrimitiveTopology topology_) {
    switch (topology_) {
        case PrimitiveTopology::eTriangleList: {
            return vk::PrimitiveTopology::eTriangleList;
        }
        case PrimitiveTopology::ePatchList: {
            return vk::PrimitiveTopology::ePatchList;
        }
        default: {
            return vk::PrimitiveTopology::eTriangleList;
        }
    }
}

vk::Format ember::engine::gfx::api::vkTranslateFormat(TextureFormat format_) {
    switch (format_) {
        case TextureFormat::eR16Uint: {
            return vk::Format::eR16Uint;
        }
        case TextureFormat::eR16G16Uint: {
            return vk::Format::eR16G16Uint;
        }
        case TextureFormat::eR32Uint: {
            return vk::Format::eR32Uint;
        }
        case TextureFormat::eR16Unorm: {
            return vk::Format::eR16Unorm;
        }
        case TextureFormat::eR16Sfloat: {
            return vk::Format::eR16Sfloat;
        }
        case TextureFormat::eR32Sfloat: {
            return vk::Format::eR32Sfloat;
        }
        case TextureFormat::eR16G16Sfloat: {
            return vk::Format::eR16G16Sfloat;
        }
        case TextureFormat::eR8Unorm: {
            return vk::Format::eR8Unorm;
        }
        case TextureFormat::eR8G8Unorm: {
            return vk::Format::eR8G8Unorm;
        }
        case TextureFormat::eR8G8B8Unorm: {
            return vk::Format::eR8G8B8Unorm;
        }
        case TextureFormat::eB8G8R8Unorm: {
            return vk::Format::eB8G8R8Unorm;
        }
        case TextureFormat::eR8G8B8A8Unorm: {
            return vk::Format::eR8G8B8A8Unorm;
        }
        case TextureFormat::eR8G8B8A8Srgb: {
            return vk::Format::eR8G8B8A8Srgb;
        }
        case TextureFormat::eB8G8R8A8Unorm: {
            return vk::Format::eB8G8R8A8Unorm;
        }
        case TextureFormat::eR16G16B16Sfloat: {
            return vk::Format::eR16G16B16Sfloat;
        }
        case TextureFormat::eR32G32B32Sfloat: {
            return vk::Format::eR32G32B32Sfloat;
        }
        case TextureFormat::eR16G16B16A16Sfloat: {
            return vk::Format::eR16G16B16A16Sfloat;
        }
        case TextureFormat::eR32G32B32A32Sfloat: {
            return vk::Format::eR32G32B32A32Sfloat;
        }
        case TextureFormat::eD32Sfloat: {
            return vk::Format::eD32Sfloat;
        }
        case TextureFormat::eD32SfloatS8Uint: {
            return vk::Format::eD32SfloatS8Uint;
        }
        default: {
            return vk::Format::eUndefined;
        }
    }
}

ember::engine::gfx::TextureFormat ember::engine::gfx::api::vkTranslateFormat(vk::Format format_) {
    switch (format_) {
        case vk::Format::eR16Uint: {
            return TextureFormat::eR16Uint;
        }
        case vk::Format::eR16G16Uint: {
            return TextureFormat::eR16G16Uint;
        }
        case vk::Format::eR32Uint: {
            return TextureFormat::eR32Uint;
        }
        case vk::Format::eR16Unorm: {
            return TextureFormat::eR16Unorm;
        }
        case vk::Format::eR16Sfloat: {
            return TextureFormat::eR16Sfloat;
        }
        case vk::Format::eR32Sfloat: {
            return TextureFormat::eR32Sfloat;
        }
        case vk::Format::eR16G16Sfloat: {
            return TextureFormat::eR16G16Sfloat;
        }
        case vk::Format::eR8Unorm: {
            return TextureFormat::eR8Unorm;
        }
        case vk::Format::eR8G8Unorm: {
            return TextureFormat::eR8G8Unorm;
        }
        case vk::Format::eR8G8B8Unorm: {
            return TextureFormat::eR8G8B8Unorm;
        }
        case vk::Format::eB8G8R8Unorm: {
            return TextureFormat::eB8G8R8Unorm;
        }
        case vk::Format::eR8G8B8A8Unorm: {
            return TextureFormat::eR8G8B8A8Unorm;
        }
        case vk::Format::eR8G8B8A8Srgb: {
            return TextureFormat::eR8G8B8A8Srgb;
        }
        case vk::Format::eB8G8R8A8Unorm: {
            return TextureFormat::eB8G8R8A8Unorm;
        }
        case vk::Format::eR16G16B16Sfloat: {
            return TextureFormat::eR16G16B16Sfloat;
        }
        case vk::Format::eR32G32B32Sfloat: {
            return TextureFormat::eR32G32B32Sfloat;
        }
        case vk::Format::eR16G16B16A16Sfloat: {
            return TextureFormat::eR16G16B16A16Sfloat;
        }
        case vk::Format::eR32G32B32A32Sfloat: {
            return TextureFormat::eR32G32B32A32Sfloat;
        }
        case vk::Format::eD32Sfloat: {
            return TextureFormat::eD32Sfloat;
        }
        case vk::Format::eD32SfloatS8Uint: {
            return TextureFormat::eD32SfloatS8Uint;
        }
        default: {
            return TextureFormat::eUndefined;
        }
    }
}

vk::DescriptorType ember::engine::gfx::api::vkTranslateBindingType(const shader::BindingType type_) {
    switch (type_) {
        case shader::BindingType::eUniformBuffer: {
            return vk::DescriptorType::eUniformBuffer;
        }
        case shader::BindingType::eStorageBuffer: {
            return vk::DescriptorType::eStorageBuffer;
        }
        case shader::BindingType::eImageSampler: {
            return vk::DescriptorType::eCombinedImageSampler;
        }
        default: {
            throw _STD runtime_error("Can not translate undefined `BindingType` to `vk::DescriptorType`.");
        }
    }
}

vk::MemoryPropertyFlags ember::engine::gfx::api::vkTranslateMemoryProperties(const MemoryProperties properties_) {

    vk::MemoryPropertyFlags flags {};

    if (properties_ & MemoryProperty::eDeviceLocal) {
        flags |= vk::MemoryPropertyFlagBits::eDeviceLocal;
    }

    if (properties_ & MemoryProperty::eHostVisible) {
        flags |= vk::MemoryPropertyFlagBits::eHostVisible;
    }

    if (properties_ & MemoryProperty::eHostCoherent) {
        flags |= vk::MemoryPropertyFlagBits::eHostCoherent;
    }

    if (properties_ & MemoryProperty::eHostCached) {
        flags |= vk::MemoryPropertyFlagBits::eHostCached;
    }

    if (properties_ & MemoryProperty::eLazilyAllocated) {
        flags |= vk::MemoryPropertyFlagBits::eLazilyAllocated;
    }

    if (properties_ & MemoryProperty::eProtected) {
        flags |= vk::MemoryPropertyFlagBits::eProtected;
    }

    return flags;
}

ember::engine::gfx::MemoryProperties ember::engine::gfx::api::vkTranslateMemoryProperties(
    const vk::MemoryPropertyFlags properties_) {

    MemoryProperties props {};

    if (properties_ & vk::MemoryPropertyFlagBits::eDeviceLocal) {
        props |= MemoryProperty::eDeviceLocal;
    }

    if (properties_ & vk::MemoryPropertyFlagBits::eHostVisible) {
        props |= MemoryProperty::eHostVisible;
    }

    if (properties_ & vk::MemoryPropertyFlagBits::eHostCoherent) {
        props |= MemoryProperty::eHostCoherent;
    }

    if (properties_ & vk::MemoryPropertyFlagBits::eHostCached) {
        props |= MemoryProperty::eHostCached;
    }

    if (properties_ & vk::MemoryPropertyFlagBits::eLazilyAllocated) {
        props |= MemoryProperty::eLazilyAllocated;
    }

    if (properties_ & vk::MemoryPropertyFlagBits::eProtected) {
        props |= MemoryProperty::eProtected;
    }

    return props;
}
