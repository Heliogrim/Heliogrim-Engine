#include "ImageAsset.hpp"

#include <Engine.Assets/AssetFactory.hpp>
#include <Engine.Assets/Types/Image.hpp>
#include <Engine.Session/Session.hpp>

using namespace ember;

ImageAsset::ImageAsset(cref<asset_guid> guid_) noexcept :
    Asset(
        guid_,
        engine::assets::Image::typeId,
        engine::Session::get()->modules().assetFactory()->createImageAsset(guid_)
    ) {}

ImageAsset::ImageAsset(cref<asset_guid> guid_, cref<string> url_) noexcept :
    Asset(guid_,
        engine::assets::Image::typeId,
        engine::Session::get()->modules().assetFactory()->createImageAsset(guid_, url_)
    ) {}

ImageAsset::~ImageAsset() noexcept = default;

bool ImageAsset::isValidType() const noexcept {
    return _typeId == engine::assets::Image::typeId;
}
