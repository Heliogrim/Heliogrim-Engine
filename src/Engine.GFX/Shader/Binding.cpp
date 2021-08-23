#include "Binding.hpp"

using namespace ember::engine::gfx::shader;
using namespace ember::engine::gfx;
using namespace ember;

[[nodiscard]] FORCE_INLINE bool isSamplerCompatible(const TextureSampler& sampler_,
    const Texture& texture_) {
    return false;
}

Binding::Binding(const BindingType type_, const id_type id_, const BindingUpdateInterval interval_,
    cref<sptr<Device>> device_, const vk::DescriptorSet& vkSet_, const vk::DescriptorSetLayout& vkSetLayout_) noexcept :
    _type(type_),
    _id(id_),
    _interval(interval_),
    _device(device_),
    _vkSet(vkSet_),
    _vkSetLayout(vkSetLayout_) {}

BindingType Binding::type() const noexcept {
    return _type;
}

u32 Binding::id() const {
    return _id;
}

BindingUpdateInterval Binding::interval() const noexcept {
    return _interval;
}

cref<sptr<Device>> Binding::device() const noexcept {
    return _device;
}

vk::DescriptorSet Binding::vkSet() const noexcept {
    return _vkSet;
}

vk::DescriptorSetLayout Binding::vkSetLayout() const noexcept {
    return _vkSetLayout;
}

void Binding::store(const Buffer& buffer_) {
    /**
     * Translate BindingType to vk::DescriptorType
     */
    vk::DescriptorType dt { vk::DescriptorType() };
    switch (_type) {
        case BindingType::eUniformBuffer: {
            dt = vk::DescriptorType::eUniformBuffer;
            break;
        }
        case BindingType::eStorageBuffer: {
            dt = vk::DescriptorType::eStorageBuffer;
            break;
        }
        default: {
            #if _DEBUG
            assert(false);
            #else
			return;
            #endif
        }
    }

    /**
     * Build Descriptor for buffer_
     */
    vk::DescriptorBufferInfo info {
        buffer_.buffer,
        0,
        buffer_.size
    };

    /**
     * Build Writer
     */
    vk::WriteDescriptorSet writer = {
        _vkSet,
        _id,
        0,
        1,
        dt,
        nullptr,
        &info,
        nullptr
    };

    /**
     * Update DescriptorSet with build Writer
     */
    _device->vkDevice().updateDescriptorSets(
        1, &writer,
        0, nullptr
    );
}

void Binding::setSampler(const TextureSampler& sampler_) noexcept {
    _sampler = sampler_;
}

void Binding::store(const Texture& texture_) {
    /**
     * Assure optimal sampler exists
     */
    if (!_sampler.vkSampler() || !isSamplerCompatible(_sampler, texture_)) {
        TextureSampler sampler {};
        /**
         * Warning: Temporary
         */
        sampler.addressModeU() = vk::SamplerAddressMode::eMirroredRepeat;
        sampler.addressModeV() = vk::SamplerAddressMode::eMirroredRepeat;
        /**
         *
         */
        sampler.lods() = texture_.mipLevels();

        sampler.setup(_device);

        /**
         * Exchange sampler
         */
        _sampler.destroy();
        _sampler = _STD move(sampler);
    }

    /**
     * Translate BindingType to vk::DescriptorType
     */
    vk::DescriptorType dt { vk::DescriptorType() };
    switch (_type) {
        case BindingType::eImageSampler: {
            dt = vk::DescriptorType::eCombinedImageSampler;
            break;
        }
        default: {
            #if _DEBUG
            assert(false);
            #else
			return;
            #endif
        }
    }

    /**
     * Build Descriptor for texture_
     */
    vk::DescriptorImageInfo info {
        _sampler.vkSampler(),
        texture_.vkView(),
        texture_.buffer()._vkLayout
    };

    /**
     * Build Writer
     */
    vk::WriteDescriptorSet writer = {
        _vkSet,
        _id,
        0,
        1,
        dt,
        &info,
        nullptr,
        nullptr
    };

    /**
     * Update DescriptorSet with build Writer
     */
    _device->vkDevice().updateDescriptorSets(
        1, &writer,
        0, nullptr
    );
}
