#include "FontAsset.hpp"

#include <Engine.Assets/AssetFactory.hpp>
#include <Engine.Assets/Types/Font.hpp>
#include <Engine.Core/Engine.hpp>
#include <Engine.Pedantic/Clone/Clone.hpp>

#include "Engine.Assets/Assets.hpp"

using namespace hg;

FontAsset::FontAsset(mref<asset_guid> guid_) noexcept :
	Asset(
		clone(guid_),
		engine::assets::Font::typeId,
		// Warning: Reference out of Scope | Use-After-Free
		engine::Engine::getEngine()->getAssets()->getFactory()->createFontAsset(clone(guid_)).get()
	) {}

FontAsset::FontAsset(mref<asset_guid> guid_, cref<string> url_) noexcept :
	Asset(
		clone(guid_),
		engine::assets::Font::typeId,
		// Warning: Reference out of Scope | Use-After-Free
		engine::Engine::getEngine()->getAssets()->getFactory()->createFontAsset(clone(guid_), url_).get()
	) {}

FontAsset::~FontAsset() noexcept = default;
