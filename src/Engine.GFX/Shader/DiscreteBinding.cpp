#include "DiscreteBinding.hpp"

#include "../Buffer/Buffer.hpp"
#include "../Buffer/VirtualBuffer.hpp"
#include "../Buffer/VirtualBufferView.hpp"
#include "../Texture/Texture.hpp"
#include "../Texture/VirtualTexture.hpp"

using namespace ember::engine::gfx::shader;
using namespace ember::engine::gfx;
using namespace ember;

[[nodiscard]] FORCE_INLINE bool isSamplerCompatible(const TextureSampler& sampler_,
    const Texture& texture_) {
    return false;
}

DiscreteBinding::DiscreteBinding() :
    _super(nullptr),
    _vkSet(),
    _sampler() {}

DiscreteBinding::DiscreteBinding(const ptr<const ShaderBinding> super_, cref<vk::DescriptorSet> vkSet_) noexcept :
    _super(super_),
    _vkSet(vkSet_),
    _sampler() {}

DiscreteBinding::DiscreteBinding(mref<DiscreteBinding> other_) noexcept :
    _super(_STD exchange(other_._super, nullptr)),
    _vkSet(_STD exchange(other_._vkSet, {})),
    _sampler(_STD exchange(other_._sampler, {})) {}

DiscreteBinding::~DiscreteBinding() {
    _sampler.destroy();
}

cref<ShaderBinding> DiscreteBinding::super() const noexcept {
    return *_super;
}

vk::DescriptorSet DiscreteBinding::vkSet() const noexcept {
    return _vkSet;
}

void DiscreteBinding::store(const ref<Buffer> buffer_) {
    /**
     * Translate BindingType to vk::DescriptorType
     */
    vk::DescriptorType dt { vk::DescriptorType() };
    switch (_super->type()) {
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
        _super->id(),
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
    _super->device()->vkDevice().updateDescriptorSets(
        1, &writer,
        0, nullptr
    );
}

void DiscreteBinding::store(const ref<VirtualBuffer> buffer_) {
    /**
     * Translate BindingType to vk::DescriptorType
     */
    vk::DescriptorType dt { vk::DescriptorType() };
    switch (_super->type()) {
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
        buffer_.vkBuffer(),
        0,
        buffer_.size()
    };

    /**
     * Build Writer
     */
    vk::WriteDescriptorSet writer = {
        _vkSet,
        _super->id(),
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
    _super->device()->vkDevice().updateDescriptorSets(
        1, &writer,
        0, nullptr
    );
}

void DiscreteBinding::store(const ref<VirtualBufferView> view_) {
    /**
     * Translate BindingType to vk::DescriptorType
     */
    vk::DescriptorType dt { vk::DescriptorType() };
    switch (_super->type()) {
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
        view_.owner()->vkBuffer(),
        0,
        view_.size()
    };

    /**
     * Build Writer
     */
    vk::WriteDescriptorSet writer = {
        _vkSet,
        _super->id(),
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
    _super->device()->vkDevice().updateDescriptorSets(
        1, &writer,
        0, nullptr
    );
}

void DiscreteBinding::setSampler(const ref<TextureSampler> sampler_) noexcept {
    _sampler = sampler_;
}

void DiscreteBinding::store(const ref<Texture> texture_) {
    storeAs(texture_, texture_.buffer()._vkLayout);
}

void DiscreteBinding::storeAs(cref<Texture> texture_, cref<vk::ImageLayout> layout_) {
    /**
     * Assure optimal sampler exists
     */
    if (!_sampler.vkSampler() || !isSamplerCompatible(_sampler, texture_)) {
        TextureSampler sampler {};
        /**
         * Warning: Temporary
         */
        if (isDepthFormat(texture_.format())) {
            sampler.addressModeU() = vk::SamplerAddressMode::eClampToEdge;
            sampler.addressModeV() = vk::SamplerAddressMode::eClampToEdge;
        } else {
            sampler.addressModeU() = vk::SamplerAddressMode::eMirroredRepeat;
            sampler.addressModeV() = vk::SamplerAddressMode::eMirroredRepeat;

            //sampler.anisotropy() = 8.0F;
        }

        /**
         *
         */
        sampler.lods() = texture_.mipLevels();

        sampler.setup(_super->device());

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
    switch (_super->type()) {
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
        layout_
    };

    /**
     * Build Writer
     */
    vk::WriteDescriptorSet writer = {
        _vkSet,
        _super->id(),
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
    _super->device()->vkDevice().updateDescriptorSets(
        1, &writer,
        0, nullptr
    );
}

void DiscreteBinding::store(const ptr<const VirtualTexture> texture_) {

    /**
     * Assure optimal sampler exists
     */
    if (!_sampler.vkSampler() || true) {
        TextureSampler sampler {};
        /**
         * Warning: Temporary
         */
        if (isDepthFormat(texture_->format())) {
            sampler.addressModeU() = vk::SamplerAddressMode::eClampToEdge;
            sampler.addressModeV() = vk::SamplerAddressMode::eClampToEdge;
        } else {
            sampler.addressModeU() = vk::SamplerAddressMode::eMirroredRepeat;
            sampler.addressModeV() = vk::SamplerAddressMode::eMirroredRepeat;
        }

        /**
         *
         */
        sampler.lods() = texture_->mipLevels();

        sampler.setup(_super->device());

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
    switch (_super->type()) {
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
        texture_->_vkImageView,
        vk::ImageLayout::eShaderReadOnlyOptimal
    };

    /**
     * Build Writer
     */
    vk::WriteDescriptorSet writer = {
        _vkSet,
        _super->id(),
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
    _super->device()->vkDevice().updateDescriptorSets(
        1, &writer,
        0, nullptr
    );

}
