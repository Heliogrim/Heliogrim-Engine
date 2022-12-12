#include "FontAsset.hpp"

#include <Engine.Assets/AssetFactory.hpp>
#include <Engine.Assets/Types/Font.hpp>
#include <Engine.Core/Engine.hpp>

#include "Engine.Assets/Assets.hpp"

using namespace ember;

FontAsset::FontAsset(cref<asset_guid> guid_) noexcept :
    Asset(
        guid_,
        engine::assets::Font::typeId,
        engine::Engine::getEngine()->getAssets()->getFactory()->createFontAsset(guid_)
    ) {}

FontAsset::FontAsset(cref<asset_guid> guid_, cref<string> url_) noexcept :
    Asset(
        guid_,
        engine::assets::Font::typeId,
        engine::Engine::getEngine()->getAssets()->getFactory()->createFontAsset(guid_, url_)
    ) {}

FontAsset::~FontAsset() noexcept = default;
