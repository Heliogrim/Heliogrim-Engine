#include "ImageAsset.hpp"

#include <Engine.Assets/AssetFactory.hpp>
#include <Engine.Assets/Types/Image.hpp>

using namespace ember;

ImageAsset::ImageAsset(cref<asset_guid> guid_) noexcept :
    Asset(
        guid_,
        engine::assets::Image::type_id,
        engine::assets::AssetFactory::get()->createImageAsset(guid_)
    ) {}

ImageAsset::~ImageAsset() noexcept = default;

bool ImageAsset::isValidType() const noexcept {
    return _typeId == engine::assets::Image::type_id;
}
