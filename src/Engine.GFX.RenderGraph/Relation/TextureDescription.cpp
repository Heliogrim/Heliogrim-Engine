#include "TextureDescription.hpp"

using namespace hg::engine::gfx::render::graph;
using namespace hg;

TextureDescription::TextureDescription() noexcept :
    Description(),
    _textureType(),
    _textureFormat(),
    _textureLayers(),
    _textureMips() {}

TextureDescription::TextureDescription(
    mref<DescriptionValue<TextureType>> textureType_,
    mref<DescriptionValue<TextureFormat>> textureFormat_,
    mref<DescriptionValue<u32>> textureLayers_,
    //mref<DescriptionValue<math::uivec3>> textureExtent_,
    mref<DescriptionValue<Vector<u32>>> textureMips_
) noexcept :
    Description(),
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
