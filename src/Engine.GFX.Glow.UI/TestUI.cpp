#include "TestUI.hpp"

#include "Widget/Panel.hpp"

#include <Engine.Common/Make.hpp>
#include <Engine.Common/String.hpp>

#include "Engine.Resource/ResourceManager.hpp"
#include "Widget/Button.hpp"
#include "Widget/Image.hpp"
#include "Widget/InputFloat.hpp"
#include "Widget/InputIntegral.hpp"
#include "Widget/InputSelect.hpp"
#include "Widget/InputSlider.hpp"
#include "Widget/InputText.hpp"
#include "Widget/InputVector.hpp"
#include "Widget/Menu.hpp"
#include "Widget/MenuButton.hpp"
#include "Widget/Text.hpp"
#include "Widget/Viewport.hpp"

#if TRUE
#include "Engine.GFX/Command/CommandBuffer.hpp"
#include "Engine.GFX/Loader/RevTextureLoader.hpp"
#include "Engine.Session/Session.hpp"
#include "Engine.GFX/Texture/TextureFactory.hpp"
#include "Engine.GFX/Graphics.hpp"
#include "Engine.Scheduler/Scheduler.hpp"
#include "Engine.Scheduler/Task/Task.hpp"
#include "Engine.Assets/Database/AssetDatabase.hpp"
#include "Engine.Assets/Database/AssetDatabaseQuery.hpp"
#include "Ember.Default/Assets/Fonts/Consolas24Latin1.hpp"
#include "Engine.Assets/Types/Font.hpp"
#include "Engine.GFX/Resource/FontResource.hpp"
#endif

using namespace ember::engine::gfx::glow::ui;
using namespace ember;

// Warning: Memory leak
static sptr<ember::engine::gfx::Texture> testTexture {};

void testLoad(cref<sptr<engine::gfx::Device>> device_) {
    // TODO:
    if (!testTexture) {
        engine::gfx::RevTextureLoader loader { engine::Session::get()->modules().graphics()->cacheCtrl() };
        testTexture = make_sptr<engine::gfx::Texture>(loader.__tmp__load({ ""sv, R"(R:\\test.ktx)" }));

        Vector<vk::ImageMemoryBarrier> imgBarriers {};
        imgBarriers.push_back({
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
        });

        auto pool = device_->graphicsQueue()->pool();
        pool->lck().acquire();
        engine::gfx::CommandBuffer iiCmd = pool->make();
        iiCmd.begin();

        /**
         * Transform
         */
        iiCmd.vkCommandBuffer().pipelineBarrier(vk::PipelineStageFlagBits::eAllCommands,
            vk::PipelineStageFlagBits::eAllCommands, vk::DependencyFlags {},
            0, nullptr,
            0, nullptr,
            static_cast<uint32_t>(imgBarriers.size()), imgBarriers.data()
        );

        iiCmd.end();
        iiCmd.submitWait();
        iiCmd.release();

        pool->lck().release();

        testTexture->buffer()._vkLayout = vk::ImageLayout::eShaderReadOnlyOptimal;

        engine::gfx::TextureFactory::get()->buildView(*testTexture);
    }
}

ember::sptr<ember::engine::gfx::glow::ui::Panel> buildTestUI(cref<sptr<engine::gfx::Device>> device_) {

    /**
     *
     */
    auto root = make_sptr<Panel>();

    ptr<engine::gfx::glow::ui::Font> defaultFont { nullptr };
    {
        using font_default_type = game::assets::font::Consolas24Latin1;

        auto* db { engine::Session::get()->modules().assetDatabase() };
        auto query { db->query(font_default_type::auto_guid()) };

        if (!query.exists()) {
            delete (new font_default_type);
        }

        assert(query.exists());

        auto* font { static_cast<ptr<engine::assets::Font>>(query.get()) };

        /**/

        auto* rm = engine::Session::get()->modules().resourceManager();
        auto* loaded = rm->loader().load(font, nullptr);

        /**/

        auto* res { static_cast<ptr<engine::gfx::FontResource>>(loaded) };
        defaultFont = res->_fontData;
    }

    /**
     *
     */
    root->setReflowType(ReflowType::eFlexCol);
    root->setReflowSpacing(ReflowSpacing::eStart);

    auto navSection = make_sptr<ReflowContainer>();
    auto contentSection = make_sptr<ReflowContainer>();

    root->add(navSection);
    root->add(contentSection);

    /**
     *
     */
    navSection->setReflowType(ReflowType::eFlexRow);
    navSection->setReflowSpacing(ReflowSpacing::eStart);
    navSection->_extent.x = 1.F;
    navSection->_extent.y = 16.F / 720.F;
    navSection->_minExtent.x = -1.F;
    navSection->_minExtent.y = 24.F;

    auto navBrandIcon = make_sptr<Image>();
    navBrandIcon->_extent.x = 0.F;
    navBrandIcon->_extent.y = 0.F;
    navBrandIcon->_minExtent.x = 32.F;
    navBrandIcon->_minExtent.y = 32.F;
    navBrandIcon->_image = testTexture;

    auto navFileButton = make_sptr<MenuButton>();
    auto navFileText = make_sptr<Text>();
    navFileButton->add(navFileText);
    navFileText->_text = "File"sv;
    navFileText->_font = defaultFont;
    navFileButton->_padding = math::vec4 { 4.F, 2.F };
    navFileButton->_margin = math::vec4 { 2.F, 6.F };

    auto navSaveAllButton = make_sptr<Button>();
    auto navSaveAllText = make_sptr<Text>();
    navSaveAllButton->add(navSaveAllText);
    navSaveAllText->_text = "Save All"sv;
    navSaveAllText->_font = defaultFont;
    navSaveAllButton->_padding = math::vec4 { 4.F, 2.F };
    navSaveAllButton->_margin = math::vec4 { 0.F };

    auto navQuitButton = make_sptr<Button>();
    auto navQuitText = make_sptr<Text>();
    navQuitButton->add(navQuitText);
    navQuitText->_text = "Quit"sv;
    navQuitText->_font = defaultFont;
    navQuitButton->_padding = math::vec4 { 4.F, 2.F };
    navQuitButton->_margin = math::vec4 { 0.F };
    [[maybe_unused]] auto qblid = navQuitButton->addOnClick([](cref<engine::input::event::MouseButtonEvent> event_) {
        engine::Scheduler::get().exec(engine::scheduler::task::make_task(engine::Session::stop));
    });

    auto navFileMenu = make_sptr<Menu>();

    navFileMenu->setVisible(false);
    navFileMenu->setRoot(root);

    root->add(navFileMenu);

    navFileButton->setMenu(navFileMenu);

    navFileMenu->add(navSaveAllButton);
    navFileMenu->add(navQuitButton);

    auto navEditButton = make_sptr<Button>();
    auto navEditText = make_sptr<Text>();
    navEditButton->add(navEditText);
    navEditText->_text = "Edit"sv;
    navEditText->_font = defaultFont;
    navEditButton->_padding = math::vec4 { 4.F, 2.F };
    navEditButton->_margin = math::vec4 { 2.F, 6.F };

    auto navWindowButton = make_sptr<Button>();
    auto navWindowText = make_sptr<Text>();
    navWindowButton->add(navWindowText);
    navWindowText->_text = "Window"sv;
    navWindowText->_font = defaultFont;
    navWindowButton->_padding = math::vec4 { 4.F, 2.F };
    navWindowButton->_margin = math::vec4 { 2.F, 6.F };

    auto navProjectButton = make_sptr<Button>();
    auto navProjectText = make_sptr<Text>();
    navProjectButton->add(navProjectText);
    navProjectText->_text = "Project"sv;
    navProjectText->_font = defaultFont;
    navProjectButton->_padding = math::vec4 { 4.F, 2.F };
    navProjectButton->_margin = math::vec4 { 2.F, 6.F };

    auto navMoreButton = make_sptr<Button>();
    auto navMoreText = make_sptr<Text>();
    navMoreButton->add(navMoreText);
    navMoreText->_text = "More"sv;
    navMoreText->_font = defaultFont;
    navMoreButton->_padding = math::vec4 { 4.F, 2.F };
    navMoreButton->_margin = math::vec4 { 2.F, 6.F };

    auto navHelpButton = make_sptr<Button>();
    auto navHelpText = make_sptr<Text>();
    navHelpButton->add(navHelpText);
    navHelpText->_text = "Help"sv;
    navHelpText->_font = defaultFont;
    navHelpButton->_padding = math::vec4 { 4.F, 2.F };
    navHelpButton->_margin = math::vec4 { 2.F, 6.F };

    //
    navSection->_background = ember::engine::color(46.F, 50.F, 58.F, 255.F);

    //
    navSection->add(navBrandIcon);
    navSection->add(navFileButton);
    navSection->add(navEditButton);
    navSection->add(navWindowButton);
    navSection->add(navProjectButton);
    navSection->add(navMoreButton);
    navSection->add(navHelpButton);

    /**
     *
     */
    contentSection->setReflowType(ReflowType::eFlexRow);
    contentSection->setReflowSpacing(ReflowSpacing::eSpaceEvenly);
    contentSection->_extent.x = 1.F;
    contentSection->_extent.y = (720.F - 16.F) / 720.F;

    auto leftSection = make_sptr<ReflowContainer>();
    contentSection->add(leftSection);

    auto mainSection = make_sptr<ReflowContainer>();
    contentSection->add(mainSection);

    #if false
    auto rightSection = make_sptr<ReflowContainer>();
    contentSection->add(rightSection);
    #endif

    #pragma region Left Section

    /**
     * Section (Left)
     */
    leftSection->setReflowType(ReflowType::eFlexCol);
    leftSection->setReflowSpacing(ReflowSpacing::eSpaceBetween);
    leftSection->_extent.x = 0.25F;
    leftSection->_extent.y = 1.F;

    auto leftTopSection = make_sptr<ReflowContainer>();
    auto leftBottomSection = make_sptr<ReflowContainer>();

    leftSection->add(leftTopSection);
    leftSection->add(leftBottomSection);

    /**
     *
     */
    leftTopSection->setReflowType(ReflowType::eFlexCol);
    leftTopSection->setReflowSpacing(ReflowSpacing::eStart);
    leftTopSection->_extent.x = 1.F;
    leftTopSection->_extent.y = 0.6F;

    /**
     *
     */
    leftBottomSection->setReflowType(ReflowType::eFlexCol);
    leftBottomSection->setReflowSpacing(ReflowSpacing::eStart);
    leftBottomSection->_extent.x = 1.F;
    leftBottomSection->_extent.y = 0.4F;

    #pragma endregion
    #pragma region Main Section

    /**
     * Section (Main)
     */
    mainSection->setReflowType(ReflowType::eFlexCol);
    mainSection->setReflowSpacing(ReflowSpacing::eStart);
    mainSection->_extent.x = 0.75F;
    mainSection->_extent.y = 1.F;

    auto mainTopSection = make_sptr<Panel>();
    auto mainBottomSection = make_sptr<ReflowContainer>();

    mainSection->add(mainTopSection);
    mainSection->add(mainBottomSection);

    /**
     *
     */
    mainTopSection->setReflowType(ReflowType::eFlexCol);
    mainTopSection->setReflowSpacing(ReflowSpacing::eStart);
    mainTopSection->_extent.x = 1.F;
    mainTopSection->_extent.y = 2.F / 3.F;

    auto viewportCtrls = make_sptr<ReflowContainer>();
    auto viewportWrapper = make_sptr<ReflowContainer>();

    mainTopSection->add(viewportCtrls);
    mainTopSection->add(viewportWrapper);

    /**
     *
     */
    viewportCtrls->setReflowType(ReflowType::eFlexRow);
    viewportCtrls->setReflowSpacing(ReflowSpacing::eSpaceAround);

    viewportCtrls->_background = ember::engine::color(48.F, 48.F, 48.F, 255.F);

    viewportCtrls->_extent.x = 1.F;
    viewportCtrls->_extent.y = 0.0F;
    viewportCtrls->_minExtent.x = -1.F;
    viewportCtrls->_minExtent.y = 24.F;

    auto playButton = make_sptr<Button>();
    auto pauseButton = make_sptr<Button>();
    auto stopButton = make_sptr<Button>();

    auto playText = make_sptr<Text>();
    playText->_font = defaultFont;
    playText->_text = "Play"sv;

    auto pauseText = make_sptr<Text>();
    pauseText->_font = defaultFont;
    pauseText->_text = "Pause"sv;

    auto stopText = make_sptr<Text>();
    stopText->_font = defaultFont;
    stopText->_text = "Stop"sv;

    viewportCtrls->add(playButton);
    viewportCtrls->add(pauseButton);
    viewportCtrls->add(stopButton);

    playButton->_background = ember::engine::color(94.F, 64.F, 64.F, 255.F);
    playButton->_margin = math::vec4 { 4.F };
    playButton->_padding = math::vec4 { 8.F, 4.F };
    playButton->add(playText);

    pauseButton->_background = ember::engine::color(64.F, 94.F, 64.F, 255.F);
    pauseButton->_margin = math::vec4 { 4.F };
    pauseButton->_padding = math::vec4 { 8.F, 4.F };
    pauseButton->add(pauseText);

    stopButton->_background = ember::engine::color(64.F, 64.F, 94.F, 255.F);
    stopButton->_margin = math::vec4 { 4.F };
    stopButton->_padding = math::vec4 { 8.F, 4.F };
    stopButton->add(stopText);

    /**
     *
     */
    viewportWrapper->setReflowType(ReflowType::eFlexRow);
    viewportWrapper->setReflowSpacing(ReflowSpacing::eSpaceAround);
    viewportWrapper->_extent.x = 1.F;
    viewportWrapper->_extent.y = 1.F;
    viewportWrapper->_reflowShrink = 1.F;

    auto viewport = make_sptr<Viewport>();

    viewportWrapper->add(viewport);
    viewport->_extent.x = 1.F;
    viewport->_extent.y = 1.F;

    /**
     *
     */
    mainBottomSection->setReflowType(ReflowType::eFlexCol);
    mainBottomSection->setReflowSpacing(ReflowSpacing::eStart);
    mainBottomSection->_extent.x = 1.F;
    mainBottomSection->_extent.y = 1.F / 3.F;

    auto assetNavBar = make_sptr<ReflowContainer>();
    auto assets = make_sptr<ReflowContainer>();

    assets->_reflowGrow = 1.F;

    mainBottomSection->add(assetNavBar);
    mainBottomSection->add(assets);

    /**
     *
     */
    assetNavBar->setReflowType(ReflowType::eFlexRow);
    assetNavBar->setReflowSpacing(ReflowSpacing::eSpaceBetween);
    assetNavBar->_extent.x = 1.F;
    assetNavBar->_extent.y = 0.F;
    assetNavBar->_minExtent.x = -1.F;
    assetNavBar->_minExtent.y = 20.F;

    auto breadcrumb = make_sptr<Button>();
    auto searchbar = make_sptr<InputText>();

    assetNavBar->add(breadcrumb);
    assetNavBar->add(searchbar);

    #pragma endregion
    #pragma region Right Section
    #if false

    /**
     * Section (Right)
     */
    rightSection->setReflowType(ReflowType::eFlexCol);
    rightSection->setReflowSpacing(ReflowSpacing::eSpaceBetween);
    rightSection->_extent.x = 0.2F;
    rightSection->_extent.y = 1.F;

    auto rightTopSection = make_sptr<ReflowContainer>();
    auto rightBottomSection = make_sptr<ReflowContainer>();

    rightSection->add(rightTopSection);
    rightSection->add(rightBottomSection);

    /**
     *
     */
    rightTopSection->setReflowType(ReflowType::eFlexCol);
    rightTopSection->setReflowSpacing(ReflowSpacing::eStart);
    rightTopSection->_extent.x = 1.F;
    rightTopSection->_extent.y = 0.6F;

    /**
     *
     */
    rightBottomSection->setReflowType(ReflowType::eFlexCol);
    rightBottomSection->setReflowSpacing(ReflowSpacing::eStart);
    rightBottomSection->_extent.x = 1.F;
    rightBottomSection->_extent.y = 0.4F;

    #endif
    #pragma endregion

    /**/
    auto* vi = new InputVector<InputFloat, InputInt, InputSelect<string>, InputSlider<float>, InputText>();
    //auto [floatIn, intIn, select, slide, text] = vi->value();
    delete vi;

    /**
     *
     */
    constexpr math::vec2 available { 1280.F, 720.F };
    constexpr math::vec2 shift { 0.F };

    root->_extent = math::vec2 { 1.F };

    UIContext ctx {
        math::fExtent2D { 1280.F, 720.F, 0.F, 0.F },
        math::fExtent2D { 1280.F, 720.F, 0.F, 0.F },
    };

    root->flow(ctx, available);
    root->shift(ctx, shift);

    return root;
}
