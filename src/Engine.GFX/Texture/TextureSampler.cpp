#include "TextureSampler.hpp"

using namespace ember::engine::gfx;
using namespace ember;

TextureSampler::TextureSampler() noexcept :
    _device(nullptr),
    _anisotropy(-1.F),
    _lods(1ui8),
    _magnification(vk::Filter::eLinear),
    _minification(vk::Filter::eLinear),
    _mipMode(vk::SamplerMipmapMode::eLinear),
    _addressModeU(vk::SamplerAddressMode::eRepeat),
    _addressModeV(vk::SamplerAddressMode::eRepeat),
    _addressModeW(vk::SamplerAddressMode::eClampToEdge),
    _vkSampler(nullptr) {}

TextureSampler::~TextureSampler() noexcept = default;

void TextureSampler::setup(cref<sptr<Device>> device_) {
    #ifdef _DEBUG
    assert(!_vkSampler);
    #endif

    _device = device_;

    const vk::SamplerCreateInfo info {
        vk::SamplerCreateFlags(),
        _magnification,
        _minification,
        _mipMode,
        _addressModeU,
        _addressModeV,
        _addressModeW,
        0.F,
        _anisotropy > 0.F,
        _anisotropy,
        {},
        {},
        0.F,
        MAX(static_cast<float>(_lods) - 1.F, 0.F),
        _borderColor,
        {}
    };
    _vkSampler = _device->vkDevice().createSampler(info);
}

void TextureSampler::destroy() {
    if (_device && _vkSampler) {
        _device->vkDevice().destroy(_vkSampler);
        _vkSampler = nullptr;
    }
}

float TextureSampler::anisotropy() const noexcept {
    return _anisotropy;
}

float& TextureSampler::anisotropy() noexcept {
    return _anisotropy;
}

u8 TextureSampler::lods() const noexcept {
    return _lods;
}

u8& TextureSampler::lods() noexcept {
    return _lods;
}

vk::Filter TextureSampler::magnification() const noexcept {
    return _magnification;
}

ref<vk::Filter> TextureSampler::magnification() noexcept {
    return _magnification;
}

vk::Filter TextureSampler::minification() const noexcept {
    return _minification;
}

ref<vk::Filter> TextureSampler::minification() noexcept {
    return _minification;
}

ref<vk::SamplerMipmapMode> TextureSampler::mipMode() noexcept {
    return _mipMode;
}

vk::SamplerAddressMode TextureSampler::addressModeU() const noexcept {
    return _addressModeU;
}

vk::SamplerAddressMode& TextureSampler::addressModeU() noexcept {
    return _addressModeU;
}

vk::SamplerAddressMode TextureSampler::addressModeV() const noexcept {
    return _addressModeV;
}

vk::SamplerAddressMode& TextureSampler::addressModeV() noexcept {
    return _addressModeV;
}

vk::SamplerAddressMode TextureSampler::addressModeW() const noexcept {
    return _addressModeW;
}

vk::SamplerAddressMode& TextureSampler::addressModeW() noexcept {
    return _addressModeW;
}

vk::BorderColor& TextureSampler::borderColor() noexcept {
    return _borderColor;
}

const vk::Sampler& TextureSampler::vkSampler() const noexcept {
    return _vkSampler;
}
