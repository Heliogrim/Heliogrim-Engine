#include "TextureAsset.hpp"

#include <Engine.Assets/AssetFactory.hpp>
#include <Engine.Assets/Assets.hpp>
#include <Engine.Assets/Types/Texture/TextureAsset.hpp>
#include <Engine.Core/Engine.hpp>
#include <Engine.Pedantic/Clone/Clone.hpp>

using namespace hg;

TextureAsset::TextureAsset(mref<asset_guid> guid_) noexcept :
    StreamableRenderableAsset(
        clone(guid_),
        engine::assets::TextureAsset::typeId,
        engine::Engine::getEngine()->getAssets()->getFactory()->createTextureAsset(clone(guid_))
    ) {}

TextureAsset::TextureAsset(
    mref<asset_guid> guid_,
    cref<asset_guid> baseImage_,
    mref<Vector<asset_guid>> images_,
    cref<math::uivec3> extent_,
    cref<TextureFormat> format_,
    cref<u32> mipLevel_,
    cref<TextureType> type_
) noexcept :
    StreamableRenderableAsset(
        clone(guid_),
        engine::assets::TextureAsset::typeId,
        engine::Engine::getEngine()->getAssets()->getFactory()->createTextureAsset(
            clone(guid_),
            baseImage_,
            std::forward<Vector<asset_guid>>(images_),
            extent_,
            format_,
            mipLevel_,
            type_
        )
    ) {}

TextureAsset::~TextureAsset() noexcept = default;

bool TextureAsset::isValidType() const noexcept {
    return _typeId == engine::assets::TextureAsset::typeId;
}
