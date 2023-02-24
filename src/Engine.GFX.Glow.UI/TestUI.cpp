#include "TestUI.hpp"

#include "Engine.Reflow/Widget/Panel.hpp"

#include <Engine.Common/Make.hpp>
#include <Engine.Core/Engine.hpp>
#include <Engine.Core/Event/SignalShutdownEvent.hpp>

#include "Editor.Core/EditorEngine.hpp"
#include "Editor.Core/EmberEditor.hpp"
#include "Editor.UI/Color/Dark.hpp"
#include "Editor.UI/Panel/SceneHierarchyPanel.hpp"
#include "Editor.UI/Panel/ObjectEditorPanel.hpp"
#include "Editor.UI/Style/Style.hpp"
#include "Ember/Ember.hpp"
#include "Ember/World.hpp"
#include "Ember/Actors/CameraActor.hpp"
#include "Ember.Default/Assets/Fonts/CascadiaCode.hpp"
#include "Ember.Default/Assets/Textures/Brand.hpp"
#include "Engine.Assets/Assets.hpp"
#include "Engine.Core/World.hpp"
#include "Engine.Core/WorldContext.hpp"
#include "Engine.Event/GlobalEventEmitter.hpp"
#include "Engine.GFX/RenderTarget.hpp"
#include "Engine.GFX/Scene/CameraModel.hpp"
#include "Engine.GFX/Swapchain/VkSwapchain.hpp"
#include "Engine.GFX/Texture/VirtualTextureView.hpp"
#include "Engine.GFX.Scene/RenderSceneManager.hpp"
#include "Engine.Resource/ResourceManager.hpp"
#include "Engine.Reflow/Widget/Button.hpp"
#include "Engine.Reflow/Widget/Image.hpp"
#include "Engine.Reflow/Widget/Menu.hpp"
#include "Engine.Reflow/Widget/Text.hpp"
#include "Engine.Reflow/Widget/Viewport.hpp"
#include "Engine.Reflow/Widget/HBox.hpp"
#include "Engine.Reflow/Widget/VBox.hpp"

#include "Engine.Reflow/Style/BoundStyleSheet.hpp"
#include "Engine.Reflow/Window/WindowManager.hpp"
#include "Engine.Scene/Scene.hpp"

#if TRUE
#include "Engine.GFX/Command/CommandBuffer.hpp"
#include "Engine.GFX/Texture/TextureFactory.hpp"
#include "Engine.GFX/Graphics.hpp"
#include "Engine.Scheduler/Async.hpp"
#include "Engine.Assets/Database/AssetDatabase.hpp"
#include "Engine.Assets/Database/AssetDatabaseQuery.hpp"
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

using namespace ember::editor::ui;
using namespace ember::engine::reflow;
using namespace ember;

// Warning: Memory leak
smr<ember::engine::gfx::TextureResource> testTexture {};
smr<ember::engine::gfx::FontResource> testFont {};

wptr<ember::engine::reflow::Widget> testFrameDisplay {};
wptr<ember::engine::reflow::Widget> testFrameTime {};

sptr<editor::ui::AssetBrowser> testAssetBrowser { nullptr };
sptr<editor::ui::ObjectEditor> testObjectEditor { nullptr };
sptr<editor::ui::SceneHierarchy> testHierarchy { nullptr };

ember::ptr<void> editorSelectedTarget { nullptr };

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

        if (!Style::get()) {
            Style::make();
        }
    }

    // TODO:
    if (testTexture.empty()) {

        const auto query = engine::Engine::getEngine()->getAssets()->getDatabase()->query(
            game::assets::texture::Brand::unstable_auto_guid()
        );

        assert(query.exists());

        auto request = static_cast<ptr<engine::assets::Texture>>(query.get());
        testTexture = engine::Engine::getEngine()->getResources()->loader().load<
            engine::assets::Texture, engine::gfx::TextureResource
        >(_STD move(request), engine::gfx::loader::TextureLoadOptions {});

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

void buildTestUI(
    cref<sptr<engine::gfx::Device>> device_,
    const non_owning_rptr<engine::reflow::Window> window_
) {
    if (!Style::get()) {
        Style::make();
    }

    /**/
    constexpr math::vec2 available { 1280.F, 720.F };
    constexpr math::vec2 shift { 0.F };

    FlowContext ctx {
        math::fExtent2D { 1280.F, 720.F, 0.F, 0.F },
        math::fExtent2D { 1280.F, 720.F, 0.F, 0.F },
    };

    StyleKeyStack stack {};

    /**/
    auto root = make_sptr<Panel>(BoundStyleSheet::make(Style::get()->getStyleSheet(Style::AdoptFlexBoxKey)));
    const ptr<engine::reflow::Font> defaultFont = getDefaultFont();

    /**/

    auto navStyle = BoundStyleSheet::make(
        StyleSheet {
            .minHeight = { true, ReflowUnit { ReflowUnitType::eAbsolute, 24.F } },
            .height = { true, ReflowUnit { ReflowUnitType::eAuto, 0.F } },
            .reflowShrink = { true, 0.F },
        }
    );
    navStyle->pushStyle({ Style::AdoptFlexBoxKey, nullptr, Style::get()->getStyleSheet(Style::AdoptFlexBoxKey) });

    auto contentStyle = BoundStyleSheet::make(
        StyleSheet {
            .reflowSpacing = { true, ReflowSpacing::eSpaceEvenly }
        }
    );
    contentStyle->pushStyle({ Style::AdoptFlexBoxKey, nullptr, Style::get()->getStyleSheet(Style::AdoptFlexBoxKey) });

    /**/

    /**
     *
     */
    auto navSection = make_sptr<HBox>(_STD move(navStyle));
    auto contentSection = make_sptr<HBox>(_STD move(contentStyle));

    root->addChild(navSection);
    root->addChild(contentSection);

    /**
     *
     */
    StyleSheet navBtnStyle {
        .padding = { true, Padding { 4.F, 2.F } },
        .margin = { true, Margin { 2.F, 6.F } },
        .color = { true, color::Dark::backgroundDefault }
    };

    StyleSheet navTxtStyle {
        .color = { true, color::Dark::grey },
        .font = { true, defaultFont },
        .fontSize = { true, 16.F }
    };

    auto navBrandIcon = make_sptr<Image>(BoundStyleSheet::make(Style::get()->getStyleSheet(Style::Icon32Key)));

    // TODO: This will break
    auto testGuard = testTexture->acquire(engine::resource::ResourceUsageFlag::eAll);
    auto testImage = *testGuard.mut();
    navBrandIcon->setImage(make_sptr<engine::gfx::ProxyTexture<non_owning_rptr>>(_STD move(testImage)), nullptr);

    StyleSheet menuBtnStyle {
        .padding = { true, Padding { 4.F, 2.F } },
        .color = { true, color::Dark::backgroundDefault }
    };

    auto navFileButton = make_sptr<Button>(BoundStyleSheet::make(navBtnStyle));
    auto navFileText = make_sptr<Text>(BoundStyleSheet::make(navTxtStyle));
    navFileButton->addChild(navFileText);
    navFileText->setText("File");

    auto navSaveAllButton = make_sptr<Button>(BoundStyleSheet::make(menuBtnStyle));
    auto navSaveAllText = make_sptr<Text>(BoundStyleSheet::make(navTxtStyle));
    navSaveAllButton->addChild(navSaveAllText);
    navSaveAllText->setText("Save All");

    auto navQuitButton = make_sptr<Button>(BoundStyleSheet::make(menuBtnStyle));
    auto navQuitText = make_sptr<Text>(BoundStyleSheet::make(navTxtStyle));
    navQuitButton->addChild(navQuitText);
    navQuitText->setText("Quit");

    [[maybe_unused]] auto qblid = navQuitButton->addOnClick(
        [](cref<engine::input::event::MouseButtonEvent> event_) {
            if (not event_._down)
                return;
            ::ember::engine::scheduler::exec(
                [] {
                    engine::Engine::getEngine()->getEmitter().emit(engine::core::SignalShutdownEvent {});
                }
            );
        }
    );

    auto navFileMenu = make_sptr<Menu>();
    auto nfmw = make_sptr<VBox>(
        BoundStyleSheet::make(
            StyleSheet {
                .color = { true, color::Dark::backgroundDefault }
            }
        )
    );

    navFileMenu->setContent(nfmw);
    navFileMenu->closeMenu();

    root->addChild(navFileMenu);

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

                WindowManager::get()->dispatch(_STD static_pointer_cast<Window, Widget>(root), focusEvent);
            }
        }
    );

    nfmw->addChild(navSaveAllButton);
    nfmw->addChild(navQuitButton);

    auto testStyle = BoundStyleSheet::make(navBtnStyle);
    testStyle->color.unset();
    testStyle->pushStyle(
        {
            Style::key_type::from("Button::Hover"),
            [](cref<sptr<Widget>> self_) {
                return self_->state().isHover();
            },
            make_sptr<StyleSheet>(
                StyleSheet {
                    .color = { true, color::Dark::raisedColor }
                }
            )
        }
    );

    auto navEditButton = make_sptr<Button>(_STD move(testStyle));
    auto navEditText = make_sptr<Text>(BoundStyleSheet::make(navTxtStyle));
    navEditButton->addChild(navEditText);
    navEditText->setText("Edit");

    auto navWindowButton = make_sptr<Button>(BoundStyleSheet::make(navBtnStyle));
    auto navWindowText = make_sptr<Text>(BoundStyleSheet::make(navTxtStyle));
    navWindowButton->addChild(navWindowText);
    navWindowText->setText("Window");

    auto navProjectButton = make_sptr<Button>(BoundStyleSheet::make(navBtnStyle));
    auto navProjectText = make_sptr<Text>(BoundStyleSheet::make(navTxtStyle));
    navProjectButton->addChild(navProjectText);
    navProjectText->setText("Project");

    auto navMoreButton = make_sptr<Button>(BoundStyleSheet::make(navBtnStyle));
    auto navMoreText = make_sptr<Text>(BoundStyleSheet::make(navTxtStyle));
    navMoreButton->addChild(navMoreText);
    navMoreText->setText("More");

    auto navHelpButton = make_sptr<Button>(BoundStyleSheet::make(navBtnStyle));
    auto navHelpText = make_sptr<Text>(BoundStyleSheet::make(navTxtStyle));
    navHelpButton->addChild(navHelpText);
    navHelpText->setText("Help");

    //
    navSection->addChild(navBrandIcon);
    navSection->addChild(navFileButton);
    navSection->addChild(navEditButton);
    navSection->addChild(navWindowButton);
    navSection->addChild(navProjectButton);
    navSection->addChild(navMoreButton);
    navSection->addChild(navHelpButton);

    /**
     *
     */
    auto lsbStyle = BoundStyleSheet::make(
        StyleSheet {
            .width = { true, ReflowUnit { ReflowUnitType::eRelative, .2F } },
            .reflowSpacing = { true, ReflowSpacing::eSpaceBetween }
        }
    );
    lsbStyle->pushStyle({ Style::AdoptFlexBoxKey, nullptr, Style::get()->getStyleSheet(Style::AdoptFlexBoxKey) });

    auto leftSection = make_sptr<VBox>(_STD move(lsbStyle));
    contentSection->addChild(leftSection);

    auto msbStyle = BoundStyleSheet::make(
        StyleSheet {
            .width = { true, ReflowUnit { ReflowUnitType::eRelative, .8F } },
            .reflowSpacing = { true, ReflowSpacing::eStart }
        }
    );
    msbStyle->pushStyle({ Style::AdoptFlexBoxKey, nullptr, Style::get()->getStyleSheet(Style::AdoptFlexBoxKey) });

    auto mainSection = make_sptr<VBox>(_STD move(msbStyle));
    contentSection->addChild(mainSection);

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

    shp->style()->minHeight = ReflowUnit { ReflowUnitType::eRelative, 1.F / 3.F };
    shp->style()->height = ReflowUnit { ReflowUnitType::eRelative, 1.F / 3.F };
    shp->style()->maxHeight = ReflowUnit { ReflowUnitType::eRelative, 1.F / 3.F };

    /**/

    const auto oep { testObjectEditor->makePanel() };
    leftSection->addChild(oep);

    oep->style()->minHeight = ReflowUnit { ReflowUnitType::eRelative, 2.F / 3.F };
    oep->style()->height = ReflowUnit { ReflowUnitType::eRelative, 2.F / 3.F };
    oep->style()->maxHeight = ReflowUnit { ReflowUnitType::eRelative, 2.F / 3.F };

    #pragma endregion
    #pragma region Main Section

    /**
     * Section (Main)
     */
    auto mtsStyle = BoundStyleSheet::make(
        StyleSheet {
            .height = { true, ReflowUnit { ReflowUnitType::eRelative, 2.F / 3.F } },
            .reflowSpacing = { true, ReflowSpacing::eStart }
        }
    );
    mtsStyle->pushStyle({ Style::AdoptFlexBoxKey, nullptr, Style::get()->getStyleSheet(Style::AdoptFlexBoxKey) });

    auto mainTopSection = make_sptr<Panel>(_STD move(mtsStyle));
    mainTopSection->style()->height = ReflowUnit { ReflowUnitType::eRelative, 2.F / 3.F };
    mainTopSection->style()->maxHeight = ReflowUnit { ReflowUnitType::eRelative, 2.F / 3.F };

    auto assetBrowsePanel = testAssetBrowser->makePanel();
    assetBrowsePanel->style()->height = ReflowUnit { ReflowUnitType::eRelative, 1.F / 3.F };
    assetBrowsePanel->style()->maxHeight = ReflowUnit { ReflowUnitType::eRelative, 1.F / 3.F };

    mainSection->addChild(mainTopSection);
    mainSection->addChild(assetBrowsePanel);

    /**
     *
     */
    auto vctrlStyle = BoundStyleSheet::make(
        StyleSheet {
            .minHeight = { true, ReflowUnit { ReflowUnitType::eAbsolute, 24.F } },
            .height = { true, ReflowUnit { ReflowUnitType::eAuto, 0.F } },
            .reflowSpacing = { true, ReflowSpacing::eSpaceAround },
            .reflowShrink = { true, 0.F },
        }
    );
    vctrlStyle->pushStyle({ Style::AdoptFlexBoxKey, nullptr, Style::get()->getStyleSheet(Style::AdoptFlexBoxKey) });

    auto vwrapStyle = BoundStyleSheet::make(
        StyleSheet {
            .reflowSpacing = { true, ReflowSpacing::eSpaceAround }
        }
    );
    vwrapStyle->pushStyle({ Style::AdoptFlexBoxKey, nullptr, Style::get()->getStyleSheet(Style::AdoptFlexBoxKey) });

    auto viewportCtrls = make_sptr<HBox>(_STD move(vctrlStyle));
    auto viewportWrapper = make_sptr<HBox>(_STD move(vwrapStyle));

    mainTopSection->addChild(viewportCtrls);
    mainTopSection->addChild(viewportWrapper);

    /**
     *
     */
    const StyleSheet cttxtStyle {
        .color = { true, color::Dark::white },
        .font = { true, defaultFont },
        .textAlign = { true, TextAlign::eCenterMiddle }
    };

    const StyleSheet ctbtnStyle {
        .padding = { true, Padding { 8.F, 4.F } },
        .margin = { true, Margin { 4.F } },
        .color = { true, color::Dark::backgroundInnerField },
    };

    auto playButton = make_sptr<Button>(BoundStyleSheet::make(ctbtnStyle));
    auto pauseButton = make_sptr<Button>(BoundStyleSheet::make(ctbtnStyle));
    auto stopButton = make_sptr<Button>(BoundStyleSheet::make(ctbtnStyle));

    auto playText = make_sptr<Text>(BoundStyleSheet::make(cttxtStyle));
    playText->setText("Play");

    auto pauseText = make_sptr<Text>(BoundStyleSheet::make(cttxtStyle));
    pauseText->setText("Pause");

    auto stopText = make_sptr<Text>(BoundStyleSheet::make(cttxtStyle));
    stopText->setText("Stop");

    viewportCtrls->addChild(playButton);
    viewportCtrls->addChild(pauseButton);
    viewportCtrls->addChild(stopButton);

    playButton->addChild(playText);
    pauseButton->addChild(pauseText);
    stopButton->addChild(stopText);

    /**
     *
     */
    auto viewport = make_sptr<Viewport>(
        BoundStyleSheet::make(
            StyleSheet {
                .width = { true, ReflowUnit { ReflowUnitType::eRelative, 1.F } },
                .height = { true, ReflowUnit { ReflowUnitType::eRelative, 1.F } },
                .color = { true, color::Dark::white }
            }
        )
    );
    viewportWrapper->addChild(viewport);

    /**/

    const auto* const gfx { editor::EditorEngine::getEngine()->getGraphics() };
    const auto* const coreSession = editor::EditorEngine::getEngine()->getPrimaryGameSession();
    const auto coreWorld { coreSession->getWorldContext()->getCurrentWorld() };
    const auto scene { coreWorld->getScene() };

    //RegisterActorClass<CameraActor>();
    coreSession->getState()->getRegistry()->getOrCreateActorPool<CameraActor>();

    auto session = EmberEditor::getSession();
    ptr<CameraActor> camera { CreateActor<CameraActor>(session) };
    auto world = GetWorld(session);

    world.addActor(camera);
    scene->update();

    viewport->setCameraActor(camera);
    viewport->flow(ctx, available, available, stack);
    viewport->rebuildView();

    const auto target { make_sptr<engine::gfx::RenderTarget>() };
    target->use(gfx->getCurrentDevice());
    target->use(gfx->getRenderer(AssocKey<string>::from("3DRenderer")).get());
    target->use(viewport->getSwapchain());

    const auto* const cc { camera->getCameraComponent() };
    const auto* const cm { *cc->getSceneNodeModels().begin() };
    target->buildPasses(static_cast<const ptr<const engine::gfx::CameraModel>>(cm)->getSceneView());

    // TODO: Should be Secondary Target
    gfx->getSceneManager()->addPrimaryTarget(target);

    viewport->addResizeListener(
        [target = wptr<engine::gfx::RenderTarget>(target)](const non_owning_rptr<engine::gfx::VkSwapchain> next_) {
            if (target.expired()) {
                return;
            }

            target.lock()->rebuildPasses(next_);
        }
    );

    /**/

    #if FALSE
    auto viewportOverlay = make_sptr<Overlay>();
    auto viewOverBox = make_sptr<HBox>(BoundStyleSheet::make(StyleSheet {
        .width = { true, ReflowUnit { ReflowUnitType::eRelative, 1.F } },
        .height = { true, ReflowUnit { ReflowUnitType::eRelative, 1.F } },
        .wrap = { true, ReflowWrap::eNoWrap },
        .reflowSpacing = { true, ReflowSpacing::eSpaceBetween },
        .color = { true, color::Dark::transparent },
    }));

    viewportWrapper->addChild(viewportOverlay);
    viewportOverlay->setContent(viewOverBox);

    {
        auto alignHelper = make_sptr<HBox>(BoundStyleSheet::make(StyleSheet {
            .reflowShrink = { true, 1.F },
            .color = { true, color::Dark::transparent },
        }));
        viewOverBox->addChild(alignHelper);

        /**/

        auto statsWrapper = make_sptr<VBox>(BoundStyleSheet::make(StyleSheet {
            .maxWidth = { true, ReflowUnit { ReflowUnitType::eRelative, 1.F } },
            .maxHeight = { true, ReflowUnit { ReflowUnitType::eRelative, 1.F } },
            .padding = { true, Padding { 8.F, 16.F } },
            .color = { true, engine::color { 0.F, 0.F, 0.F, 0.2F } },
        }));
        viewOverBox->addChild(statsWrapper);

        /**/

        const StyleSheet stxtStyle {
            .minWidth = { true, ReflowUnit { ReflowUnitType::eAbsolute, 156.F } },
            .color = { true, color::Dark::white },
            .font = { true, defaultFont },
            .fontSize = { true, 16.F },
            .textAlign = { true, TextAlign::eRightTop }
        };

        auto sceneName = make_sptr<Text>(BoundStyleSheet::make(stxtStyle));
        sceneName->setText("Test Scene");

        statsWrapper->addChild(sceneName);

        auto sceneCalcTime = make_sptr<Text>(BoundStyleSheet::make(stxtStyle));
        sceneCalcTime->setText("15.42 ms");

        testFrameTime = sceneCalcTime;
        statsWrapper->addChild(sceneCalcTime);

        auto sceneFrames = make_sptr<Text>(BoundStyleSheet::make(stxtStyle));
        sceneFrames->setText("63 FPS");

        testFrameDisplay = sceneFrames;
        statsWrapper->addChild(sceneFrames);
    }
    #endif

    #pragma endregion

    /**
     *
     */
    root->flow(ctx, available, available, stack);
    root->shift(ctx, shift);

    /**/

    window_->setContent(root);
}

const ember::ptr<ember::engine::reflow::Font> getDefaultFont() {
    using font_default_type = game::assets::font::CascadiaCode;

    if (testFont.empty()) {

        auto* db { engine::Engine::getEngine()->getAssets()->getDatabase() };
        auto query { db->query(font_default_type::unstable_auto_guid()) };

        if (!query.exists()) {
            delete (new font_default_type);
        }

        assert(query.exists());

        auto* font { static_cast<ptr<engine::assets::Font>>(query.get()) };

        /**/

        testFont = engine::Engine::getEngine()->getResources()->loader().load<
            engine::assets::Font,
            engine::gfx::FontResource
        >(
            _STD move(font),
            engine::gfx::loader::FontLoadOptions {
                .ranges = { engine::reflow::BasicLatin, engine::reflow::Latin1Supplement },
                .glyphs = {},
                //
                .fontSizes = { 12ui32, 16ui32, 24ui32 }
            }
        );
    }

    return testFont->acquire(engine::resource::ResourceUsageFlag::eAll).mut();
}

/**/

#include "Ember/StaticGeometryComponent.hpp"

void storeActorMapping() {
    testObjectEditor->storeObjectMapper("ember::Actor"_typeId, make_uptr<ObjectValueMapper<::ember::Actor>>());
    testObjectEditor->storeObjectMapper(
        ::ember::StaticGeometryComponent::typeId,
        make_uptr<ObjectValueMapper<::ember::StaticGeometryComponent>>()
    );
}

void loadActorMappingExp(const type_id typeId_, cref<sptr<ObjectEditorPanel>> panel_) {
    if (not editorSelectedTarget) {
        __debugbreak();
        return;
    }

    panel_->setEditorTarget(typeId_, editorSelectedTarget);
}

void storeEditorSelectedTarget(const ember::ptr<ember::Actor> target_) {
    if (!testObjectEditor) {
        return;
    }

    const auto& panels { testObjectEditor->_panels };
    if (panels.empty()) {
        return;
    }

    const auto panel { panels.front().lock() };
    editorSelectedTarget = target_;
    loadActorMappingExp("ember::Actor"_typeId, panel);
}

void storeEditorSelectedTarget(const ember::ptr<ember::ActorComponent> target_) {
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
        sources.push_back(_STD move(sve));
    }

    panel->setHierarchyTarget<SceneViewEntry>("sptr<SceneViewEntry>"_typeId, sources);
}
