#include "EditorUI.hpp"
#include "Module/EditorUI.hpp"

#include <Editor.Assets.Default/Textures/Default.hpp>
#include <Editor.Core/EditorEngine.hpp>
#include <Editor.Main/Module/Editor.hpp>
#include <Editor.UI/Helper/AssetBrowserHelper.hpp>
#include <Editor.UI/Modules/AssetBrowser.hpp>
#include <Editor.UI/Modules/ObjectEditor.hpp>
#include <Editor.UI/Modules/SceneHierarchy.hpp>
#include <Editor.UI.Init/EditorInit.hpp>
#include <Engine.Common/Make.hpp>
#include <Engine.GFX.Loader/Texture/TextureLoadOptions.hpp>

#include "Temp.hpp"

using namespace hg::editor::ui;
using namespace hg::engine::core;
using namespace hg::engine;
using namespace hg;

editor::EditorUI::EditorUI(ref<EditorEngine> engine_) :
	SubModule(std::addressof(engine_)),
	_dependencies(
		{
			SubModuleDependency {
				EditorDepKey,
				SubModuleOrder::eSuccessor,
				true
			}
		}
	) {}

editor::EditorUI::~EditorUI() = default;

engine::core::DependencyKey editor::EditorUI::moduleKey() const noexcept {
	return EditorUIDepKey;
}

cref<CompactSet<engine::core::SubModuleDependency>> editor::EditorUI::dependencies() const noexcept {
	return _dependencies;
}

void editor::EditorUI::setup() {}

void editor::EditorUI::start() {

	AssetBrowserHelper::make();

	_assetBrowser = make_sptr<AssetBrowser>();
	_objectEditor = make_sptr<ObjectEditor>();
	_sceneHierarchy = make_sptr<SceneHierarchy>();

	/**/

	storeActorMapping(*this);
	storeSceneHierarchyMeta(*this);

	/**/

	initEditor(*this);
}

void editor::EditorUI::stop() {

	if (not placeholderHolder.empty()) {
		placeholderHolder.reset();
	}

	_sceneHierarchy.reset();
	_objectEditor.reset();
	_assetBrowser.reset();

	AssetBrowserHelper::destroy();
}

void editor::EditorUI::destroy() {}

sptr<AssetBrowser> editor::EditorUI::getAssetBrowser() const noexcept {
	return _assetBrowser;
}

sptr<ObjectEditor> editor::EditorUI::getObjectEditor() const noexcept {
	return _objectEditor;
}

sptr<SceneHierarchy> editor::EditorUI::getSceneHierarchy() const noexcept {
	return _sceneHierarchy;
}

#include <Editor.Assets.Default/Fonts/Default.hpp>
#include <Engine.Assets.System/IAssetRegistry.hpp>
#include <Engine.Assets/Assets.hpp>
#include <Engine.Assets.Type/Texture/Font.hpp>
#include <Engine.GFX.Loader/Font/FontLoader.hpp>
#include <Engine.GFX.Loader/Font/FontLoadOptions.hpp>
#include <Engine.GFX.Loader/Font/FontResource.hpp>
#include <Engine.GFX.Loader/Texture/TextureLoader.hpp>
#include <Engine.GFX.Loader/Texture/TextureResource.hpp>
#include <Engine.Resource/ResourceManager.hpp>

nmpt<engine::assets::Font> editor::EditorUI::getDefaultFont() const noexcept {

	auto registry = _engine->getAssets()->getRegistry();
	auto asset = registry->findAssetByGuid(assets::font::default_font_guid);
	::hg::assertrt(asset != nullptr);

	/**/

	// Problem: We have to move the parametrized loading behaviour into the consumption position instead of preloading the stuff.
	std::ignore = engine::Engine::getEngine()->getResources()->loader().load<
		engine::assets::Font, engine::gfx::FontResource
	>(
		Cast<engine::assets::Font>(asset->get()),
		engine::gfx::loader::FontLoadOptions {
			.ranges = { engine::reflow::BasicLatin, engine::reflow::Latin1Supplement },
			.glyphs = {},
			.fontSizes = { 12uL, 16uL, 24uL }
		}
	);

	return Cast<engine::assets::Font>(asset->get());
}

smr<::hg::engine::gfx::TextureResource> editor::EditorUI::getPlaceholderImage() const noexcept {

	if (placeholderHolder.empty()) {
		const auto asset = assets::texture::get_default_brand();
		::hg::assertrt(asset != nullptr);

		auto* request = Cast<engine::assets::TextureAsset>(asset->get());
		placeholderHolder = _engine->getResources()->loader().load<
			engine::assets::TextureAsset, engine::gfx::TextureResource
		>(std::move(request), engine::gfx::loader::TextureLoadOptions {});
	}
	return placeholderHolder;
}
