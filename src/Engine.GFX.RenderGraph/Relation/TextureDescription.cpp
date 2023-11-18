#include "TextureDescription.hpp"

#include <Engine.GFX/Texture/Texture.hpp>
#include <Engine.GFX/Texture/TextureView.hpp>
#include <Engine.GFX/Texture/VirtualTexture.hpp>
#include <Engine.GFX/Texture/VirtualTextureView.hpp>

using namespace hg::engine::gfx::render::graph;
using namespace hg;

TextureDescription::TextureDescription() noexcept :
    InheritMeta(),
    _textureType(),
    _textureFormat(),
    _textureLayers(),
    _textureMips() {}

TextureDescription::TextureDescription(
    mref<DescriptionValue<TextureType>> textureType_,
    mref<DescriptionValue<TextureFormat>> textureFormat_,
    mref<DescriptionValue<u32>> textureLayers_,
    //mref<DescriptionValue<math::uivec3>> textureExtent_,
    mref<DescriptionValue<ActiveMipBitMask>> textureMips_
) noexcept :
    InheritMeta(),
    _textureType(textureType_),
    _textureFormat(textureFormat_),
    _textureLayers(textureLayers_),
    //_textureExtent(textureExtent_),
    _textureMips(_STD move(textureMips_)) {}

TextureDescription::~TextureDescription() noexcept = default;

bool TextureDescription::isValueCompatible(const non_owning_rptr<const Description> other_) const noexcept {

    const auto& other = *static_cast<const ptr<const this_type>>(other_);

    if (
        _textureType.mode == DescriptionValueMatchingMode::eInvariant &&
        _textureType.value != other._textureType.value
    ) {
        return false;
    }

    if (
        _textureFormat.mode == DescriptionValueMatchingMode::eInvariant &&
        _textureFormat.value != other._textureFormat.value
    ) {
        return false;
    }

    if (
        _textureLayers.mode == DescriptionValueMatchingMode::eInvariant &&
        _textureLayers.value != other._textureLayers.value
    ) {
        return false;
    }

    if (
        _textureMips.mode == DescriptionValueMatchingMode::eInvariant &&
        _textureMips.value != other._textureMips.value
    ) {
        return false;
    }

    return true;
}

bool TextureDescription::isValidObject(const nmpt<Texture> texture_) const noexcept {

    bool valid = true;
    if (_textureType.mode == DescriptionValueMatchingMode::eInvariant) {
        valid &= _textureType.value == texture_->type();
    }

    if (_textureFormat.mode == DescriptionValueMatchingMode::eInvariant) {
        valid &= _textureFormat.value == texture_->format();
    }

    if (_textureLayers.mode == DescriptionValueMatchingMode::eInvariant) {
        valid &= _textureLayers.value == texture_->layer();
    }

    if (_textureMips.mode == DescriptionValueMatchingMode::eInvariant) {

        for (auto i = 0; i <= ActiveMipBitMask::maxMipLevel(); ++i) {

            const auto active = _textureMips.value.active(i);
            const auto stored = texture_->mipLevels() > i;

            valid &= (active == stored);
        }
    }

    return valid;
}

bool TextureDescription::isValidObject(const nmpt<TextureView> textureView_) const noexcept {

    bool valid = true;
    if (_textureType.mode == DescriptionValueMatchingMode::eInvariant) {
        valid &= _textureType.value == textureView_->owner()->type();
    }

    if (_textureFormat.mode == DescriptionValueMatchingMode::eInvariant) {
        valid &= _textureFormat.value == textureView_->owner()->format();
    }

    if (_textureLayers.mode == DescriptionValueMatchingMode::eInvariant) {
        valid &= _textureLayers.value == textureView_->layers();
    }

    if (_textureMips.mode == DescriptionValueMatchingMode::eInvariant) {

        for (auto i = 0; i <= ActiveMipBitMask::maxMipLevel(); ++i) {

            const auto active = _textureMips.value.active(i);
            const auto stored = textureView_->mipLevels() > i;

            valid &= (active == stored);
        }
    }

    return valid;
}

bool TextureDescription::isValidObject(const nmpt<VirtualTexture> texture_) const noexcept {

    bool valid = true;
    if (_textureType.mode == DescriptionValueMatchingMode::eInvariant) {
        valid &= _textureType.value == texture_->type();
    }

    if (_textureFormat.mode == DescriptionValueMatchingMode::eInvariant) {
        valid &= _textureFormat.value == texture_->format();
    }

    if (_textureLayers.mode == DescriptionValueMatchingMode::eInvariant) {
        valid &= _textureLayers.value == texture_->layers();
    }

    if (_textureMips.mode == DescriptionValueMatchingMode::eInvariant) {

        for (auto i = 0; i <= ActiveMipBitMask::maxMipLevel(); ++i) {

            const auto active = _textureMips.value.active(i);
            const auto stored = texture_->mipLevels() > i;

            valid &= (active == stored);
        }
    }

    return valid;
}

bool TextureDescription::isValidObject(const nmpt<VirtualTextureView> textureView_) const noexcept {

    bool valid = true;
    if (_textureType.mode == DescriptionValueMatchingMode::eInvariant) {
        valid &= _textureType.value == textureView_->type();
    }

    if (_textureFormat.mode == DescriptionValueMatchingMode::eInvariant) {
        valid &= _textureFormat.value == textureView_->format();
    }

    if (_textureLayers.mode == DescriptionValueMatchingMode::eInvariant) {
        valid &= _textureLayers.value == textureView_->layers();
    }

    if (_textureMips.mode == DescriptionValueMatchingMode::eInvariant) {

        for (auto i = 0; i <= ActiveMipBitMask::maxMipLevel(); ++i) {

            const auto active = _textureMips.value.active(i);
            const auto stored = textureView_->mipLevels() > i;

            valid &= (active == stored);
        }
    }

    return valid;
}
