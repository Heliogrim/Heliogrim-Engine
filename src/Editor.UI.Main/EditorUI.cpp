#include "EditorUI.hpp"
#include "Module/EditorUI.hpp"

#include <Editor.Assets.Default/Textures/Default.hpp>
#include <Editor.Core/EditorEngine.hpp>
#include <Editor.Main/Module/Editor.hpp>
#include <Editor.UI.View/Scene/SceneEditorController.hpp>
#include <Editor.UI/Helper/AssetBrowserHelper.hpp>
#include <Editor.UI.Init/EditorInit.hpp>
#include <Editor.UI.Rx/Subject.hpp>
#include <Editor.UI.Theme/EditorTheme.hpp>
#include <Engine.Common/Make.hpp>
#include <Engine.Reflow/Reflow.hpp>
#include <Engine.Reflow/Module/Reflow.hpp>

using namespace hg::editor::ui;
using namespace hg::engine::core;
using namespace hg::engine;
using namespace hg;

Opt<ref<editor::EditorUI>> editor::EditorUI::_cached = None;

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
	),
	_uiModel(),
	_uiServices(),
	_sceneEditorController() {
	EditorUI::_cached = Some(*this);
}

editor::EditorUI::~EditorUI() {
	EditorUI::_cached = None;
}

Opt<ref<editor::EditorUI>> editor::EditorUI::getEditorUI() {
	return EditorUI::_cached;
}

engine::core::DependencyKey editor::EditorUI::moduleKey() const noexcept {
	return EditorUIDepKey;
}

cref<CompactSet<engine::core::SubModuleDependency>> editor::EditorUI::dependencies() const noexcept {
	return _dependencies;
}

void editor::EditorUI::setup() {

	auto reflowModule = _engine->getModules().getSubModule(ReflowDepKey);
	::hg::assertrt(reflowModule != nullptr);

	auto& reflow = static_cast<ref<Reflow>>(*reflowModule);
	setupEditorTheme(reflow.getTheming());

}

void editor::EditorUI::start() {

	AssetBrowserHelper::make();

	/**/

	_uiServices.assetBrowserService = make_sptr<service::AssetBrowserService>();
	_uiServices.contextService = make_sptr<service::ContextService>();
	_uiServices.propertyService = make_sptr<service::PropertyService>();
	_uiServices.sceneHierarchyService = make_sptr<service::SceneHierarchyService>();

	/**/

	initEditor(*this);
}

void editor::EditorUI::stop() {

	_sceneEditorController.reset();

	_uiServices.sceneHierarchyService.reset();
	_uiServices.propertyService.reset();
	_uiServices.contextService.reset();
	_uiServices.assetBrowserService.reset();

	AssetBrowserHelper::destroy();
}

void editor::EditorUI::destroy() {}

ref<EditorUiModel> editor::EditorUI::getEditorModel() noexcept {
	return _uiModel;
}

ref<const EditorUiModel> editor::EditorUI::getEditorModel() const noexcept {
	return _uiModel;
}

ref<const EditorUiServices> editor::EditorUI::getEditorServices() const noexcept {
	return _uiServices;
}

#include <Editor.Assets.Default/Fonts/Default.hpp>
#include <Engine.Assets.System/IAssetRegistry.hpp>
#include <Engine.Assets/Assets.hpp>
#include <Engine.Assets.Type/Texture/Font.hpp>
#include <Engine.GFX.Loader/Font/FontLoader.hpp>
#include <Engine.GFX.Loader/Font/FontLoadOptions.hpp>
#include <Engine.GFX.Loader/Font/FontResource.hpp>
#include <Engine.Resource/ResourceManager.hpp>

ref<const AssetGuid> editor::EditorUI::getDefaultFontGuid() const noexcept {
	return assets::font::default_font_guid;
}

nmpt<engine::assets::Font> editor::EditorUI::getDefaultFont() const noexcept {

	auto registry = _engine->getAssets()->getRegistry();
	auto asset = registry->findAssetByGuid(assets::font::default_font_guid);
	::hg::assertrt(asset != None);

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
