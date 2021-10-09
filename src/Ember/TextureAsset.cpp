#include "TextureAsset.hpp"

#include <Engine.Assets/AssetFactory.hpp>
#include <Engine.Assets/Types/Texture.hpp>

using namespace ember;

TextureAsset::TextureAsset(cref<asset_guid> guid_) noexcept :
    StreamableRenderableAsset(
        guid_,
        engine::assets::Texture::type_id,
        engine::assets::AssetFactory::get()->createTextureAsset(guid_)
    ) {}

TextureAsset::TextureAsset(
    cref<asset_guid> guid_,
    cref<asset_guid> baseImage_,
    mref<vector<asset_guid>> images_,
    cref<math::uivec3> extent_,
    cref<TextureFormat> format_,
    cref<u32> mipLevel_,
    cref<TextureType> type_
) noexcept :
    StreamableRenderableAsset(
        guid_,
        engine::assets::Texture::type_id,
        engine::assets::AssetFactory::get()->createTextureAsset(
            guid_,
            baseImage_,
            _STD forward<vector<asset_guid>>(images_),
            extent_,
            format_,
            mipLevel_,
            type_
        )
    ) {}

TextureAsset::~TextureAsset() noexcept = default;

bool TextureAsset::isValidType() const noexcept {
    return _typeId == engine::assets::Texture::type_id;
}
