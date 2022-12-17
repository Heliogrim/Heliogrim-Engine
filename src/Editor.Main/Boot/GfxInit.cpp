#include "GfxInit.hpp"

#include <Editor.Core/EmberEditor.hpp>
#include <Ember/Actor.hpp>
#include <Ember/ActorInitializer.hpp>
#include <Ember/Inbuilt.hpp>
#include <Ember/World.hpp>
#include <Ember/Actors/CameraActor.hpp>
#include <Engine.Common/Math/Extent.hpp>
#include <Engine.Core/Engine.hpp>
#include <Engine.Core/Session.hpp>
#include <Engine.Core/World.hpp>
#include <Engine.GFX.Scene/RenderSceneManager.hpp>
#include <Engine.GFX/Graphics.hpp>
#include <Engine.GFX/RenderTarget.hpp>
#include <Engine.GFX.Glow.UI/Renderer/UIRenderer.hpp>
#include <Engine.GFX/Scene/CameraModel.hpp>
#include <Engine.GFX/Surface/Surface.hpp>
#include <Engine.GFX/Surface/SurfaceManager.hpp>
#include <Engine.GFX/Swapchain/VkSurfaceSwapchain.hpp>
#include <Engine.Platform/NativeWindow.hpp>
#include <Engine.Scene/Scene.hpp>
#include <Engine.Platform/Platform.hpp>

#include "Engine.GFX.Scene/View/SceneView.hpp"
#include "Engine.Reflow/Window/WindowManager.hpp"

using namespace ember::editor::boot;
using namespace ember::engine;
using namespace ember;

void editor::boot::initGfx() {

    const auto* const engine { EmberEditor::getEngine() };
    const auto editor { EmberEditor::getEditorSession() };
    const auto game { EmberEditor::getSession() };

    /**/

    const auto* const gfx { engine->getGraphics() };
    auto* const gfxScenes { gfx->getSceneManager() };
    auto* const gfxSurfaces { gfx->getSurfaceManager() };

    /**/

    auto* const platform { engine->getPlatform() };
    auto window = await(Future { platform->makeNativeWindow("Ember Editor"sv, math::iExtent2D { 1280, 720 }) });

    auto* const surface { gfxSurfaces->makeSurface(_STD move(window)) };
    surface->setup();

    const auto swapchain { make_sptr<gfx::VkSurfaceSwapchain>(surface) };
    swapchain->setup(gfx->getCurrentDevice());

    [[maybe_unused]] const auto boundSurface = surface->setSwapchain(swapchain);
    assert(boundSurface);

    /**/

    const auto renderTarget = make_sptr<gfx::RenderTarget>();
    renderTarget->use(gfx->getCurrentDevice());
    renderTarget->use(gfx->getRenderer(AssocKey<string>::from("UIRenderer")).get());
    renderTarget->use(swapchain.get());
    renderTarget->use(surface);

    renderTarget->setActive(false);
    gfxScenes->addPrimaryTarget(renderTarget);

    /**/

    //RegisterActorClass<CameraActor>();
    static_cast<const ptr<core::Session>>(editor.unwrap().get())->getState()->getRegistry()->getOrCreateActorPool<
        CameraActor>();

    /**/

    auto initializer { editor.getActorInitializer() };
    auto camera { CreateActor<CameraActor>(editor) };

    auto world { GetWorld(editor) };
    world.addActor(camera);

    /**/

    // Warning: Unsafe forwarded behaviour -> Should wait for scene tick
    static_cast<const ptr<core::World>>(world.unwrap().get())->getScene()->update();

    /**/

    const auto* const comp = camera->getCameraComponent();
    const auto& models { comp->getSceneNodeModels() };

    assert(not models.empty());
    const auto* const model = *(models.begin());

    const auto* const camModel { static_cast<const ptr<const gfx::CameraModel>>(model) };
    auto* const sceneView { camModel->getSceneView() };

    renderTarget->buildPasses(sceneView);

    engine::reflow::WindowManager::get()->requestWindow(
        "Ember Editor"sv,
        math::ivec2 { 1280, 720 },
        {},
        sceneView->getScene()
    );
}
