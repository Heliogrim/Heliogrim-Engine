#include "TestUI.hpp"

#include "Engine.Reflow/Widget/Panel.hpp"

#include <Engine.Common/Make.hpp>
#include <Engine.Core/Engine.hpp>
#include <Engine.Core/Event/SignalShutdownEvent.hpp>
#include <Engine.Render.Scene.Model/CameraModel.hpp>
#include <Heliogrim/Components/DirectionalLightComponent.hpp>
#include <Heliogrim/Components/PointLightComponent.hpp>

#include "Editor.Core/EditorEngine.hpp"
#include "Editor.Core/HeliogrimEditor.hpp"
#include "Editor.UI/Color/Dark.hpp"
#include "Editor.UI/Panel/SceneHierarchyPanel.hpp"
#include "Editor.UI/Panel/ObjectEditorPanel.hpp"
#include "Heliogrim/Heliogrim.hpp"
#include "Heliogrim/World.hpp"
#include "Heliogrim/Actors/CameraActor.hpp"
#include "Heliogrim.Default/Assets/Fonts/CascadiaCode.hpp"
#include "Heliogrim.Default/Assets/Textures/Brand.hpp"
#include "Engine.Assets/Assets.hpp"
#include "Engine.Core/World.hpp"
#include "Engine.Core/WorldContext.hpp"
#include "Engine.Event/GlobalEventEmitter.hpp"
#include "Engine.GFX/RenderTarget.hpp"
#include "Engine.GFX/Swapchain/VkSwapchain.hpp"
#include "Engine.GFX/Texture/SparseTextureView.hpp"
#include "Engine.GFX.Scene/RenderSceneManager.hpp"
#include "Engine.Resource/ResourceManager.hpp"
#include "Engine.Reflow/Widget/Button.hpp"
#include "Engine.Reflow/Widget/Image.hpp"
#include "Engine.Reflow/Widget/Menu.hpp"
#include "Engine.Reflow/Widget/Text.hpp"
#include "Engine.Reflow/Widget/Viewport.hpp"
#include "Engine.Reflow/Widget/HorizontalPanel.hpp"
#include "Engine.Reflow/Widget/VerticalPanel.hpp"
#include "Engine.Reflow/Widget/Overlay.hpp"

#include "Engine.Reflow/Window/WindowManager.hpp"
#include "Engine.Scene/Scene.hpp"
#include "Engine.ACS/Registry.hpp"
#include "Engine.ACS/Pool.hpp"
#include "Engine.ACS/Storage.hpp"
#include "Engine.Assets.System/IAssetRegistry.hpp"
#include <Engine.Render.Scene/RenderSceneSystem.hpp>
#include <Engine.GFX.Scene/View/SceneView.hpp>

#if TRUE
#include "Engine.GFX/Command/CommandBuffer.hpp"
#include "Engine.GFX/Texture/TextureFactory.hpp"
#include "Engine.GFX/Graphics.hpp"
#include "Engine.Scheduler/Async.hpp"
#include "Engine.Assets/Types/Font.hpp"
#include "Engine.Resource/LoaderManager.hpp"
#include "Engine.GFX.Loader/Texture/Traits.hpp"
#include "Engine.GFX.Loader/Font/Traits.hpp"
#include "Engine.GFX.Loader/Font/FontResource.hpp"
#endif

#if TRUE
#include "Editor.UI/Panel/AssetBrowserPanel.hpp"
#include "Editor.UI/Modules/AssetBrowser.hpp"
#include "Editor.UI/Helper/AssetBrowserHelper.hpp"
#include "Editor.UI/Modules/SceneHierarchy.hpp"
#include "Editor.UI/Modules/SceneHierarchy/HierarchyGenerator.hpp"
#include "Editor.UI/Modules/SceneHierarchy/HierarchyResolver.hpp"
#include "Editor.UI/Modules/SceneHierarchy/SceneViewEntry.hpp"
#endif

using namespace hg::editor::ui;
using namespace hg::engine::reflow;
using namespace hg;

// Warning: Memory leak
smr<hg::engine::gfx::TextureResource> testTexture {};
smr<hg::engine::gfx::FontResource> testFont {};

wptr<hg::engine::reflow::Widget> testFrameDisplay {};
wptr<hg::engine::reflow::Widget> testFrameTime {};

sptr<editor::ui::AssetBrowser> testAssetBrowser { nullptr };
sptr<editor::ui::ObjectEditor> testObjectEditor { nullptr };
sptr<editor::ui::SceneHierarchy> testHierarchy { nullptr };

hg::ptr<void> editorSelectedTarget { nullptr };

void storeActorMapping();

void storeHierarchyMeta();

void loadActorMappingExp(cref<sptr<ObjectEditorPanel>> panel_);

void destroyLoaded() {

	if (not testAssetBrowser) {
		return;
	}

	testHierarchy.reset();
	testObjectEditor.reset();
	testAssetBrowser.reset();
	editor::ui::AssetBrowserHelper::destroy();

	if (!testTexture.empty()) {
		testTexture.reset();
	}
}

void testLoad(cref<sptr<engine::gfx::Device>> device_) {
	if (!testAssetBrowser) {
		editor::ui::AssetBrowserHelper::make();
		testAssetBrowser = make_sptr<editor::ui::AssetBrowser>();
		testObjectEditor = make_sptr<editor::ui::ObjectEditor>();
		testHierarchy = make_sptr<editor::ui::SceneHierarchy>();

		storeActorMapping();
		storeHierarchyMeta();
	}

	// TODO:
	if (testTexture.empty()) {

		const auto asset = engine::Engine::getEngine()->getAssets()->getRegistry()->findAssetByGuid(
			game::assets::texture::Brand::unstable_auto_guid()
		);

		assert(asset != nullptr);

		auto* request = Cast<engine::assets::TextureAsset>(asset.get());
		testTexture = engine::Engine::getEngine()->getResources()->loader().load<
			engine::assets::TextureAsset, engine::gfx::TextureResource
		>(std::move(request), engine::gfx::loader::TextureLoadOptions {});

		/*
		testTexture = make_sptr<engine::gfx::Texture>(loader.__tmp__load({ ""sv, R"(R:\\test.ktx)" }));

		Vector<vk::ImageMemoryBarrier> imgBarriers {};
		imgBarriers.push_back(
		    {
		        vk::AccessFlags {},
		        vk::AccessFlagBits::eShaderRead,
		        vk::ImageLayout::eTransferSrcOptimal,
		        vk::ImageLayout::eShaderReadOnlyOptimal,
		        VK_QUEUE_FAMILY_IGNORED,
		        VK_QUEUE_FAMILY_IGNORED,
		        testTexture->buffer().image(),
		        vk::ImageSubresourceRange {
		            vk::ImageAspectFlagBits::eColor,
		            0,
		            testTexture->mipLevels(),
		            0,
		            testTexture->layer()
		        }
		    }
		);

		auto pool = device_->graphicsQueue()->pool();
		pool->lck().acquire();
		engine::gfx::CommandBuffer iiCmd = pool->make();

		iiCmd.begin();
		iiCmd.vkCommandBuffer().pipelineBarrier(
		    vk::PipelineStageFlagBits::eAllCommands,
		    vk::PipelineStageFlagBits::eAllCommands,
		    vk::DependencyFlags {},
		    0,
		    nullptr,
		    0,
		    nullptr,
		    static_cast<uint32_t>(imgBarriers.size()),
		    imgBarriers.data()
		);

		iiCmd.end();
		iiCmd.submitWait();
		iiCmd.release();

		pool->lck().release();

		testTexture->buffer()._vkLayout = vk::ImageLayout::eShaderReadOnlyOptimal;

		engine::gfx::TextureFactory::get()->buildView(*testTexture);

		*/
	}
}

/**/

static void configureMainViewport(
	sptr<VerticalPanel> parent_
);

static void configureMainGraph(
	sptr<VerticalPanel> parent_
);

/**/

void buildTestUI(
	cref<sptr<engine::gfx::Device>> device_,
	const non_owning_rptr<engine::reflow::Window> window_
) {

	/**/
	constexpr math::vec2 available { 1280.F, 720.F };
	constexpr math::vec2 shift { 0.F };

	FlowContext ctx {
		math::fExtent2D { 1280.F, 720.F, 0.F, 0.F },
		math::fExtent2D { 1280.F, 720.F, 0.F, 0.F },
	};

	/**/
	auto root = make_sptr<VerticalPanel>();
	root->attr.width.setValue({ ReflowUnitType::eRelative, 1.F });
	root->attr.maxWidth.setValue({ ReflowUnitType::eRelative, 1.F });
	root->attr.height.setValue({ ReflowUnitType::eRelative, 1.F });
	root->attr.maxHeight.setValue({ ReflowUnitType::eRelative, 1.F });

	const auto defaultFont = getDefaultFont();

	/**/

	/**
	 *
	 */
	auto navSection = make_sptr<HorizontalPanel>();
	navSection->attr.width.setValue({ ReflowUnitType::eRelative, 1.F });
	navSection->attr.maxWidth.setValue({ ReflowUnitType::eRelative, 1.F });
	navSection->attr.minHeight.setValue({ ReflowUnitType::eAbsolute, 24.F });
	navSection->attr.height.setValue({ ReflowUnitType::eAuto });
	navSection->attr.maxHeight.setValue({ ReflowUnitType::eRelative, 1.F });
	navSection->attr.colGap.setValue(4.F);
	navSection->attr.padding.setValue(Padding { 2.F, 6.F });
	navSection->attr.flexGrow.setValue(1.F);

	auto contentSection = make_sptr<HorizontalPanel>();
	contentSection->attr.width.setValue({ ReflowUnitType::eRelative, 1.F });
	contentSection->attr.maxWidth.setValue({ ReflowUnitType::eRelative, 1.F });
	contentSection->attr.minHeight.setValue({ ReflowUnitType::eAbsolute, 24.F });
	contentSection->attr.height.setValue({ ReflowUnitType::eRelative, 1.F });
	contentSection->attr.maxHeight.setValue({ ReflowUnitType::eRelative, 1.F });
	contentSection->attr.justify.setValue(ReflowSpacing::eSpaceEvenly);
	contentSection->attr.flexGrow.setValue(1.F);
	contentSection->attr.flexShrink.setValue(1.F);

	root->addChild(navSection);
	root->addChild(contentSection);

	/**
	 *
	 */

	auto navBrandIcon = make_sptr<Image>();
	ReflowUnit abs32 { ReflowUnitType::eAbsolute, 32.F };
	navBrandIcon->attr.minWidth.setValue(abs32);
	navBrandIcon->attr.width.setValue(abs32);
	navBrandIcon->attr.maxWidth.setValue(abs32);
	navBrandIcon->attr.minHeight.setValue(abs32);
	navBrandIcon->attr.height.setValue(abs32);
	navBrandIcon->attr.maxHeight.setValue(abs32);

	// TODO: This will break
	auto testGuard = testTexture->acquire(engine::resource::ResourceUsageFlag::eAll);
	navBrandIcon->setImage(*testGuard.imm(), nullptr);

	auto navFileButton = make_sptr<Button>();
	navFileButton->attributes().padding.setValue(Padding { 4.F, 2.F });
	//navFileButton->attr.color.setValue(color::Dark::darkRed);

	auto navFileText = make_sptr<Text>();
	navFileText->attr.font.setValue(*defaultFont);
	navFileText->attr.fontSize.setValue(16.F);
	navFileText->attr.color.setValue(color::Dark::grey);

	navFileButton->setChild(navFileText);
	navFileText->setText("File");

	auto navSaveAllButton = make_sptr<Button>();
	navSaveAllButton->attributes().padding.setValue(Padding { 4.F, 2.F });
	//navFileButton->attr.color.setValue(color::Dark::backgroundDefault);

	auto navSaveAllText = make_sptr<Text>();
	navSaveAllText->attr.font.setValue(*defaultFont);
	navSaveAllText->attr.fontSize.setValue(16.F);
	navSaveAllText->attr.color.setValue(color::Dark::grey);

	navSaveAllButton->setChild(navSaveAllText);
	navSaveAllText->setText("Save All");

	auto navQuitButton = make_sptr<Button>();
	navQuitButton->attributes().padding.setValue(Padding { 4.F, 2.F });
	//navQuitButton->attr.color.setValue(color::Dark::backgroundDefault);

	auto navQuitText = make_sptr<Text>();
	navQuitText->attr.font.setValue(*defaultFont);
	navQuitText->attr.fontSize.setValue(16.F);
	navQuitText->attr.color.setValue(color::Dark::grey);

	navQuitButton->setChild(navQuitText);
	navQuitText->setText("Quit");

	[[maybe_unused]] auto qblid = navQuitButton->addOnClick(
		[](cref<engine::input::event::MouseButtonEvent> event_) {
			if (not event_._down)
				return;
			::hg::engine::scheduler::exec(
				[] {
					engine::Engine::getEngine()->getEmitter().emit(engine::core::SignalShutdownEvent {});
				}
			);
		}
	);

	auto navFileMenu = make_sptr<Menu>();
	auto nfmw = make_sptr<VerticalPanel>();

	navFileMenu->setContent(nfmw);
	navFileMenu->closeMenu();

	//root->addChild(navFileMenu);

	[[maybe_unused]] auto handle = navFileButton->addOnClick(
		[self = wptr<Widget> { navFileButton }, nfm = wptr<Menu> { navFileMenu }](auto event_) {
			if (not event_._down || nfm.expired()) {
				return;
			}

			auto lck { nfm.lock() };
			if (lck) {
				lck->openMenu();

				FocusEvent focusEvent { lck };
				auto root { self.lock()->root() };

				WindowManager::get()->dispatch(std::static_pointer_cast<Window, Widget>(root), focusEvent);
			}
		}
	);

	nfmw->addChild(navSaveAllButton);
	nfmw->addChild(navQuitButton);

	auto navEditButton = make_sptr<Button>();
	navEditButton->attributes().padding.setValue(Padding { 4.F, 2.F });
	//navEditButton->attr.color.setValue(color::Dark::darkRed);

	auto navEditText = make_sptr<Text>();
	navEditText->attr.font.setValue(*defaultFont);
	navEditText->attr.fontSize.setValue(16.F);
	navEditText->attr.color.setValue(color::Dark::grey);
	navEditButton->setChild(navEditText);
	navEditText->setText("Edit");

	auto navWindowButton = make_sptr<Button>();
	navWindowButton->attributes().padding.setValue(Padding { 4.F, 2.F });
	//navWindowButton->attr.color.setValue(color::Dark::darkRed);

	auto navWindowText = make_sptr<Text>();
	navWindowText->attr.font.setValue(*defaultFont);
	navWindowText->attr.fontSize.setValue(16.F);
	navWindowText->attr.color.setValue(color::Dark::grey);

	navWindowButton->setChild(navWindowText);
	navWindowText->setText("Window");

	auto navProjectButton = make_sptr<Button>();
	navProjectButton->attributes().padding.setValue(Padding { 4.F, 2.F });
	//navProjectButton->attr.color.setValue(color::Dark::darkRed);

	auto navProjectText = make_sptr<Text>();
	navProjectText->attr.font.setValue(*defaultFont);
	navProjectText->attr.fontSize.setValue(16.F);
	navProjectText->attr.color.setValue(color::Dark::grey);

	navProjectButton->setChild(navProjectText);
	navProjectText->setText("Project");

	auto navMoreButton = make_sptr<Button>();
	navMoreButton->attributes().padding.setValue(Padding { 4.F, 2.F });
	//navMoreButton->attr.color.setValue(color::Dark::darkRed);

	auto navMoreText = make_sptr<Text>();
	navMoreText->attr.font.setValue(*defaultFont);
	navMoreText->attr.fontSize.setValue(16.F);
	navMoreText->attr.color.setValue(color::Dark::grey);

	navMoreButton->setChild(navMoreText);
	navMoreText->setText("More");

	auto navHelpButton = make_sptr<Button>();
	navHelpButton->attributes().padding.setValue(Padding { 4.F, 2.F });
	//navHelpButton->attr.color.setValue(color::Dark::darkRed);

	auto navHelpText = make_sptr<Text>();
	navHelpText->attr.font.setValue(*defaultFont);
	navHelpText->attr.fontSize.setValue(16.F);
	navHelpText->attr.color.setValue(color::Dark::grey);

	navHelpButton->setChild(navHelpText);
	navHelpText->setText("Help");

	//
	navSection->addChild(navBrandIcon);
	navSection->addChild(navFileButton);
	navSection->addChild(navEditButton);
	navSection->addChild(navWindowButton);
	navSection->addChild(navProjectButton);
	navSection->addChild(navMoreButton);
	navSection->addChild(navHelpButton);

	/**/

	auto leftSection = make_sptr<VerticalPanel>();
	leftSection->attr.width.setValue({ ReflowUnitType::eRelative, .2F });
	leftSection->attr.maxWidth.setValue({ ReflowUnitType::eRelative, 1.F });
	leftSection->attr.height.setValue({ ReflowUnitType::eRelative, 1.F });
	leftSection->attr.maxHeight.setValue({ ReflowUnitType::eRelative, 1.F });
	leftSection->attr.justify.setValue(ReflowSpacing::eSpaceBetween);
	leftSection->attr.flexGrow.setValue(1.F);
	leftSection->attr.flexShrink.setValue(1.F);

	auto mainSection = make_sptr<VerticalPanel>();
	mainSection->attr.width.setValue({ ReflowUnitType::eRelative, .8F });
	mainSection->attr.maxWidth.setValue({ ReflowUnitType::eRelative, 1.F });
	mainSection->attr.height.setValue({ ReflowUnitType::eRelative, 1.F });
	mainSection->attr.maxHeight.setValue({ ReflowUnitType::eRelative, 1.F });
	mainSection->attr.justify.setValue(ReflowSpacing::eStart);
	mainSection->attr.flexGrow.setValue(1.F);
	mainSection->attr.flexShrink.setValue(1.F);

	contentSection->addChild(leftSection);
	contentSection->addChild(mainSection);

	/**/

	#if false
    auto rightSection = make_sptr<ReflowContainer>();
    contentSection->add(rightSection);
	#endif

	#pragma region Left Section

	/**
	 * Section (Left)
	 */
	const auto shp { testHierarchy->makePanel() };
	leftSection->addChild(shp);

	shp->attr.minHeight.setValue({ ReflowUnitType::eRelative, 1.F / 3.F });
	shp->attr.height.setValue({ ReflowUnitType::eRelative, 1.F / 3.F });
	shp->attr.maxHeight.setValue({ ReflowUnitType::eRelative, 1.F / 3.F });

	/**/

	const auto oep { testObjectEditor->makePanel() };
	leftSection->addChild(oep);

	oep->attr.minHeight.setValue({ ReflowUnitType::eRelative, 2.F / 3.F });
	oep->attr.height.setValue({ ReflowUnitType::eRelative, 2.F / 3.F });
	oep->attr.maxHeight.setValue({ ReflowUnitType::eRelative, 2.F / 3.F });

	#pragma endregion
	#pragma region Main Section

	/**
	 * Section (Main)
	 */

	auto mainTopSection = make_sptr<VerticalPanel>();
	mainTopSection->attr.width.setValue({ ReflowUnitType::eRelative, 1.F });
	mainTopSection->attr.maxWidth.setValue({ ReflowUnitType::eRelative, 1.F });
	mainTopSection->attr.height.setValue({ ReflowUnitType::eRelative, 2.F / 3.F });
	mainTopSection->attr.maxHeight.setValue({ ReflowUnitType::eRelative, 2.F / 3.F });
	mainTopSection->attr.flexGrow.setValue(1.F);
	mainTopSection->attr.flexShrink.setValue(1.F);

	auto assetBrowsePanel = testAssetBrowser->makePanel();
	assetBrowsePanel->attr.height.setValue({ ReflowUnitType::eRelative, 1.F / 3.F });
	assetBrowsePanel->attr.maxHeight.setValue({ ReflowUnitType::eRelative, 1.F / 3.F });

	mainSection->addChild(mainTopSection);
	mainSection->addChild(assetBrowsePanel);

	if (true) {
		configureMainViewport(mainTopSection);
	} else {
		configureMainGraph(mainTopSection);
	}

	#pragma endregion

	/**/

	window_->setContent(root);

	/**/

	{
		auto* session = static_cast<ptr<engine::core::Session>>(GetSession().unwrap().get());
		auto* registry = session->getState()->getRegistry();

		Vector<ptr<Actor>> actors {};

		auto* const pool = registry->getOrCreateActorPool<Actor>();
		const auto& storage = pool->__get_storage();

		// Error: Dual-Layer Iterators are broken
		const auto end = storage.cend();
		for (auto iter = storage.cbegin(); iter != end; ++iter) {
			if (iter.get_key() != invalid_actor_guid) {
				const auto& actor = iter.get_value();
				actors.push_back(std::addressof(const_cast<ref<Actor>>(actor)));
			}
		}

		if (not actors.empty()) {
			::storeEditorSelectedTarget(actors.front());
			storeHierarchyActor(actors);
		}
	}
}

nmpt<engine::assets::Font> getDefaultFont() {
	using font_default_type = game::assets::font::CascadiaCode;

	auto registry = engine::Engine::getEngine()->getAssets()->getRegistry();
	auto asset = registry->findAssetByGuid(font_default_type::unstable_auto_guid());

	if (asset == nullptr) {
		delete (new font_default_type);
		asset = registry->findAssetByGuid(font_default_type::unstable_auto_guid());

		std::ignore = engine::Engine::getEngine()->getResources()->loader().load<
			engine::assets::Font, engine::gfx::FontResource
		>(
			Cast<engine::assets::Font>(asset.get()),
			engine::gfx::loader::FontLoadOptions {
				.ranges = { engine::reflow::BasicLatin, engine::reflow::Latin1Supplement },
				.glyphs = {},
				//
				.fontSizes = { 12uL, 16uL, 24uL }
			}
		);
	}

	/**/

	return Cast<engine::assets::Font>(asset.get());
}

nmpt<hg::engine::reflow::Font> getDefaultFontInstance() {
	using font_default_type = game::assets::font::CascadiaCode;

	if (testFont.empty()) {

		const auto font = getDefaultFont();
		testFont = engine::Engine::getEngine()->getResources()->loader().load<
			engine::assets::Font,
			engine::gfx::FontResource
		>(
			std::move(font.get()),
			engine::gfx::loader::FontLoadOptions {
				.ranges = { engine::reflow::BasicLatin, engine::reflow::Latin1Supplement },
				.glyphs = {},
				//
				.fontSizes = { 12uL, 16uL, 24uL }
			}
		);
	}

	return testFont->acquire(engine::resource::ResourceUsageFlag::eAll).mut();
}

/**/

#include "Heliogrim/StaticGeometryComponent.hpp"

void storeActorMapping() {
	testObjectEditor->storeObjectMapper("hg::Actor"_typeId, make_uptr<ObjectValueMapper<::hg::Actor>>());
	testObjectEditor->storeObjectMapper(
		::hg::StaticGeometryComponent::typeId,
		make_uptr<ObjectValueMapper<::hg::StaticGeometryComponent>>()
	);
	testObjectEditor->storeObjectMapper(
		::hg::PointLightComponent::typeId,
		make_uptr<ObjectValueMapper<::hg::PointLightComponent>>()
	);
	testObjectEditor->storeObjectMapper(
		::hg::DirectionalLightComponent::typeId,
		make_uptr<ObjectValueMapper<::hg::DirectionalLightComponent>>()
	);
}

void loadActorMappingExp(const type_id typeId_, cref<sptr<ObjectEditorPanel>> panel_) {
	if (not editorSelectedTarget) {
		__debugbreak();
		return;
	}

	panel_->setEditorTarget(typeId_, editorSelectedTarget);
}

void storeEditorSelectedTarget(const hg::ptr<hg::Actor> target_) {
	if (!testObjectEditor) {
		return;
	}

	const auto& panels { testObjectEditor->_panels };
	if (panels.empty()) {
		return;
	}

	const auto panel { panels.front().lock() };
	editorSelectedTarget = target_;
	loadActorMappingExp("hg::Actor"_typeId, panel);
}

void storeEditorSelectedTarget(const hg::ptr<hg::ActorComponent> target_) {
	if (!testObjectEditor) {
		return;
	}

	const auto& panels { testObjectEditor->_panels };
	if (panels.empty()) {
		return;
	}

	const auto panel { panels.front().lock() };
	editorSelectedTarget = target_;
	loadActorMappingExp(target_->getTypeId(), panel);
}

/**/

void storeHierarchyMeta() {
	testHierarchy->storeGenerator("sptr<SceneViewEntry>"_typeId, make_uptr<HierarchyGenerator<sptr<SceneViewEntry>>>());
	testHierarchy->storeResolver("sptr<SceneViewEntry>"_typeId, make_uptr<HierarchyResolver<sptr<SceneViewEntry>>>());
}

void storeHierarchyActor(cref<Vector<ptr<Actor>>> targets_) {
	if (!testHierarchy) {
		return;
	}

	const auto& panels { testHierarchy->_panels };
	if (panels.empty()) {
		return;
	}

	const auto panel { panels.front().lock() };

	Vector<sptr<SceneViewEntry>> sources {};

	for (const auto& actor : targets_) {
		auto sve { make_sptr<SceneViewEntry>() };
		sve->storeTarget<Actor>(actor);
		sources.push_back(std::move(sve));
	}

	panel->setHierarchyTarget<SceneViewEntry>("sptr<SceneViewEntry>"_typeId, sources);
}

void configureMainViewport(
	sptr<VerticalPanel> parent_
) {

	const auto defaultFont = getDefaultFont();

	/**/

	auto viewportCtrls = make_sptr<HorizontalPanel>();
	viewportCtrls->attr.width.setValue({ ReflowUnitType::eRelative, 1.F });
	viewportCtrls->attr.maxWidth.setValue({ ReflowUnitType::eRelative, 1.F });
	viewportCtrls->attr.minHeight.setValue({ ReflowUnitType::eAbsolute, 24.F });
	viewportCtrls->attr.height.setValue({ ReflowUnitType::eAuto });
	viewportCtrls->attr.maxHeight.setValue({ ReflowUnitType::eRelative, 1.F });
	viewportCtrls->attr.padding.setValue(Padding { 4.F, 4.F });
	viewportCtrls->attr.justify.setValue(ReflowSpacing::eSpaceAround);
	viewportCtrls->attr.colGap.setValue(8.F);
	viewportCtrls->attr.flexGrow.setValue(1.F);

	auto viewportWrapper = make_sptr<HorizontalPanel>();
	viewportWrapper->attr.width.setValue({ ReflowUnitType::eRelative, 1.F });
	viewportWrapper->attr.maxWidth.setValue({ ReflowUnitType::eRelative, 1.F });
	viewportWrapper->attr.height.setValue({ ReflowUnitType::eRelative, 1.F });
	viewportWrapper->attr.maxHeight.setValue({ ReflowUnitType::eRelative, 1.F });
	viewportWrapper->attr.justify.setValue(ReflowSpacing::eSpaceAround);
	viewportWrapper->attr.flexGrow.setValue(1.F);
	viewportWrapper->attr.flexShrink.setValue(1.F);

	parent_->addChild(viewportCtrls);
	parent_->addChild(viewportWrapper);

	/**/

	auto playButton = make_sptr<Button>();
	playButton->attributes().padding.setValue(Padding { 8.F, 4.F });

	auto pauseButton = make_sptr<Button>();
	pauseButton->attributes().padding.setValue(Padding { 8.F, 4.F });

	auto stopButton = make_sptr<Button>();
	stopButton->attributes().padding.setValue(Padding { 8.F, 4.F });

	auto playText = make_sptr<Text>();
	playText->attr.font.setValue(*defaultFont);
	playText->attr.textAlign.setValue(TextAlign::eCenterMiddle);
	playText->attr.color.setValue(color::Dark::white);
	playText->setText("Play");

	auto pauseText = make_sptr<Text>();
	pauseText->attr.font.setValue(*defaultFont);
	pauseText->attr.textAlign.setValue(TextAlign::eCenterMiddle);
	pauseText->attr.color.setValue(color::Dark::white);
	pauseText->setText("Pause");

	auto stopText = make_sptr<Text>();
	stopText->attr.font.setValue(*defaultFont);
	stopText->attr.textAlign.setValue(TextAlign::eCenterMiddle);
	stopText->attr.color.setValue(color::Dark::white);
	stopText->setText("Stop");

	viewportCtrls->addChild(playButton);
	viewportCtrls->addChild(pauseButton);
	viewportCtrls->addChild(stopButton);

	playButton->setChild(playText);
	pauseButton->setChild(pauseText);
	stopButton->setChild(stopText);

	/**
	 *
	 */
	auto viewport = make_sptr<Viewport>();
	viewport->attr.width.setValue({ ReflowUnitType::eRelative, 1.F });
	viewport->attr.height.setValue({ ReflowUnitType::eRelative, 1.F });

	viewportWrapper->addChild(viewport);

	/**/

	const auto* const gfx { editor::EditorEngine::getEngine()->getGraphics() };
	const auto coreSession = editor::EditorEngine::getEngine()->getPrimaryGameSession();
	const auto coreWorld { coreSession->getWorldContext()->getCurrentWorld() };
	const auto scene { coreWorld->getScene() };

	//RegisterActorClass<CameraActor>();
	coreSession->getState()->getRegistry()->getOrCreateActorPool<CameraActor>();

	auto session = HeliogrimEditor::getSession();
	ptr<CameraActor> camera { CreateActor<CameraActor>(session) };
	auto world = GetWorld(session); {
		cref<math::Transform> tf = camera->getRootComponent()->getWorldTransform();
		const_cast<ref<math::Transform>>(tf).setLocation(math::Location { 0.F, 0.F, -5.F });
		//const_cast<ref<math::Transform>>(tf).setLocation(math::Location { 0.F, 1.8F, 0.F });
	}
	world.addActor(camera);

	/**/

	viewport->setViewportTarget("Test-Renderer"sv, world, camera);
	viewport->rebuildView();

	/**/

	/**/

	viewport->addResizeListener(
		[instance = viewport.get()](mref<smr<engine::gfx::Swapchain>> prev_, mref<smr<engine::gfx::Swapchain>> next_) {

			if (not instance->hasMountedRenderTarget()) {
				return;
			}

			auto* const manager = engine::Engine::getEngine()->getGraphics()->getSceneManager();
			manager->transitionToTarget(std::move(prev_), std::move(next_), nullptr);
		}
	);

	/**/

	auto viewportOverlay = make_sptr<Overlay>();
	auto viewOverBox = make_sptr<HorizontalPanel>(); {
		auto& attr = viewOverBox->attr;
		attr.width.setValue(ReflowUnitType::eAuto, 0.F);
		attr.height.setValue(ReflowUnitType::eAuto, 0.F);
		attr.style.setValue(
			{
				.backgroundColor = engine::color { 1.F, 1.F, 1.F, 0.F }
			}
		);
	}

	viewportWrapper->addChild(viewportOverlay);
	viewportOverlay->setContent(viewOverBox); {
		auto statsWrapper = make_sptr<VerticalPanel>(); {
			auto& attr = statsWrapper->attr;
			attr.width.setValue(ReflowUnitType::eAuto, 0.F);
			attr.height.setValue(ReflowUnitType::eAuto, 0.F);
			attr.style.setValue(
				{
					.backgroundColor = engine::color { 1.F, 1.F, 1.F, 0.F }
				}
			);
		}

		viewOverBox->addChild(statsWrapper);

		/**/

		auto sceneName = make_sptr<Text>();
		sceneName->attr.font.setValue(*defaultFont);
		sceneName->attr.fontSize.setValue(16.F);
		sceneName->attr.color.setValue(engine::color { 229.F, 190.F, 1.F, 255.F });
		sceneName->setText("Test Scene");

		statsWrapper->addChild(sceneName);

		auto sceneCalcTime = make_sptr<Text>();
		sceneCalcTime->attr.font.setValue(*defaultFont);
		sceneCalcTime->attr.fontSize.setValue(16.F);
		sceneCalcTime->attr.color.setValue(engine::color { 229.F, 190.F, 1.F, 255.F });
		sceneCalcTime->setText("15.42 ms");

		testFrameTime = sceneCalcTime;
		statsWrapper->addChild(sceneCalcTime);

		auto sceneFrames = make_sptr<Text>();
		sceneFrames->attr.font.setValue(*defaultFont);
		sceneFrames->attr.fontSize.setValue(16.F);
		sceneFrames->attr.color.setValue(engine::color { 229.F, 190.F, 1.F, 255.F });
		sceneFrames->setText("63 FPS");

		testFrameDisplay = sceneFrames;
		statsWrapper->addChild(sceneFrames);
	}

}

#include <Editor.UI/Widget/Board/Whiteboard.hpp>
#include <Editor.UI/Widget/Board/Board.hpp>
#include <Editor.UI/Widget/Board/BoardNode.hpp>
#include <Editor.GFX.Graphs/Node/Input/SubGraphInputNode.hpp>
#include <Editor.GFX.Graphs/Node/Math/ScalarSubMathNode.hpp>
#include <Editor.GFX.Graphs/Node/Math/ScalarPowMathNode.hpp>
#include <Editor.GFX.Graphs/Node/Output/SubGraphOutputNode.hpp>

void configureMainGraph(
	sptr<VerticalPanel> parent_
) {

	auto whiteboard = make_sptr<Whiteboard>();
	auto board = make_sptr<Board>();

	whiteboard->setBoard(board);
	parent_->addChild(whiteboard);

	/**/

	auto node0 = make_sptr<editor::gfx::graph::nodes::SubGraphInputNode>();
	node0->setName("Out Input Node");
	node0->regenerate();
	auto test0 = BoardNode::make(node0);

	auto node1 = make_sptr<editor::gfx::graph::nodes::ScalarSubMathNode>();
	node1->setName("Useless Operation #1");
	node1->regenerate();
	auto test1 = BoardNode::make(node1);

	auto node2 = make_sptr<editor::gfx::graph::nodes::ScalarPowMathNode>();
	node2->setName("Useless Operation #2");
	node2->regenerate();
	auto test2 = BoardNode::make(node2);

	auto node3 = make_sptr<editor::gfx::graph::nodes::SubGraphOutputNode>();
	node3->setName("Out Output Node");
	node3->regenerate();
	auto test3 = BoardNode::make(node3);

	test0->setBoardPosition(math::vec2 { -240.F, 0.F });
	//test0->style()->margin.attr.x = -240.F;
	test1->setBoardPosition(math::vec2 { 0.F, -50.F });
	//test1->style()->margin.attr.y = -50.F;
	test2->setBoardPosition(math::vec2 { 0.F, 50.F });
	//test2->style()->margin.attr.y = 50.F;
	test3->setBoardPosition(math::vec2 { 240.F, 0.F });
	//test3->style()->margin.attr.x = 240.F;

	board->addChild(test0);
	board->addChild(test1);
	board->addChild(test2);
	board->addChild(test3);
}
