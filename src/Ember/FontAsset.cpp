#include "FontAsset.hpp"

#include <Engine.Assets/AssetFactory.hpp>
#include <Engine.Assets/Types/Font.hpp>
#include <Engine.Session/Session.hpp>

using namespace ember;

FontAsset::FontAsset(cref<asset_guid> guid_) noexcept :
    Asset(
        guid_,
        engine::assets::Font::typeId,
        engine::Session::get()->modules().assetFactory()->createFontAsset(guid_)
    ) {}

FontAsset::FontAsset(cref<asset_guid> guid_, cref<string> url_) noexcept :
    Asset(
        guid_,
        engine::assets::Font::typeId,
        engine::Session::get()->modules().assetFactory()->createFontAsset(guid_, url_)
    ) {}

FontAsset::~FontAsset() noexcept = default;
