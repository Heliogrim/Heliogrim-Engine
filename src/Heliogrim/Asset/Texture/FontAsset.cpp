#include <Engine.Assets.Type/Texture/Font.hpp>
/**/
#include "FontAsset.hpp"

#include <Engine.Asserts/Todo.hpp>
#include <Engine.Assets/AssetFactory.hpp>
#include <Engine.Assets/Assets.hpp>
#include <Engine.Core/Engine.hpp>
#include <Engine.Pedantic/Clone/Clone.hpp>
#include <Engine.Reflect/IsType.hpp>

using namespace hg;

FontAsset::FontAsset(mref<AssetGuid> guid_) noexcept :
	Asset(
		clone(guid_),
		engine::assets::Font::typeId,
		// Warning: Reference out of Scope | Use-After-Free
		*engine::Engine::getEngine()->getAssets()->getFactory()->createFontAsset(clone(guid_))
	) {}

FontAsset::FontAsset(mref<AssetGuid> guid_, cref<string> url_) noexcept :
	Asset(
		clone(guid_),
		engine::assets::Font::typeId,
		// Warning: Reference out of Scope | Use-After-Free
		*engine::Engine::getEngine()->getAssets()->getFactory()->createFontAsset(clone(guid_), url_)
	) {}

FontAsset::~FontAsset() noexcept = default;

bool FontAsset::isValidType() const noexcept {
	return internal() != nullptr && IsType<::hg::engine::assets::Font>(*internal());
}

cref<string> FontAsset::url() const noexcept {
	::hg::todo_panic();
}
