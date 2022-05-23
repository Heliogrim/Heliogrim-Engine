#include "Texture.hpp"

using namespace ember::engine::assets;

Texture::Texture(cref<asset_guid> guid_) :
    Data(guid_, Texture::typeId) {}

Texture::Texture(cref<asset_guid> guid_, cref<asset_guid> baseImage_, mref<Vector<asset_guid>> images_,
    cref<math::uivec3> extent_, cref<gfx::TextureFormat> format_, cref<u32> mipLevel_, cref<gfx::TextureType> type_) :
    Data(guid_, Texture::typeId),
    _baseImage(baseImage_),
    _images(images_),
    _extent(extent_),
    _format(format_),
    _mipLevel(mipLevel_),
    _textureType(type_) {}
