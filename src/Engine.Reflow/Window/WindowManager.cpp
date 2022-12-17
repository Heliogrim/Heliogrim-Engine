#include "WindowManager.hpp"

#include <Ember/Inbuilt.hpp>
#include <Ember/Future.hpp>
#include <Engine.Core/Engine.hpp>
#include <Engine.GFX/RenderTarget.hpp>
#include <Engine.GFX/Surface/Surface.hpp>
#include <Engine.Logging/Logger.hpp>

#include "BoundWindow.hpp"
#include "Window.hpp"
#include "Engine.GFX/Graphics.hpp"
#include "Engine.GFX.Scene/View/SceneView.hpp"
#include "Engine.GFX/Surface/SurfaceManager.hpp"
#include "Engine.GFX/Swapchain/VkSurfaceSwapchain.hpp"
#include "Engine.GFX.Scene/RenderSceneManager.hpp"
#include "Engine.Platform/Platform.hpp"

using namespace ember::engine::reflow;
using namespace ember;

sptr<WindowManager> WindowManager::_instance = nullptr;

cref<sptr<WindowManager::this_type>> WindowManager::get() {
    return _instance;
}

cref<sptr<WindowManager::this_type>> WindowManager::make() {
    if (!_instance) {
        _instance = sptr<WindowManager>(new WindowManager());
    }
    return _instance;
}

void WindowManager::destroy() {
    _instance.reset();
}

WindowManager::WindowManager() = default;

WindowManager::~WindowManager() {
    tidy();
}

void WindowManager::tidy() {

    if (_windows.empty()) {
        return;
    }

    /**/

    Vector<sptr<Window>> left {};
    left.reserve(_windows.size());

    for (const auto& bound : _windows) {
        left.push_back(bound->window);
    }

    /**/

    for (auto&& entry : left) {
        destroyWindow(_STD move(entry));
    }

}

non_owning_rptr<engine::scene::IRenderScene> WindowManager::resolveScene(cref<sptr<Window>> window_) {

    const auto iter = _STD find_if(_windows.begin(), _windows.end(), [window_](cref<uptr<BoundWindow>> entry_) {
        return entry_.get()->window == window_;
    });

    if (iter == _windows.end()) {
        return nullptr;
    }

    return iter->get()->sceneView->getScene();
}

void WindowManager::destroyWindow(mref<sptr<Window>> window_) {

    const auto wnd { _STD move(window_) };

    const auto iter {
        _STD find_if(_windows.begin(), _windows.end(), [wnd](const auto& entry_) {
            return entry_.get()->window == wnd;
        })
    };

    if (iter == _windows.end()) {
        // Shared pointer will destroy itself and we don't hold a reference
        return;
    }

    auto bound = _STD move(*iter);
    _windows.erase(iter);

    /**/

    auto* const platform { Engine::getEngine()->getPlatform() };
    const auto* const gfx { Engine::getEngine()->getGraphics() };

    #ifdef _DEBUG
    if (wnd.use_count() > 2) {
        // We have 2 references, one is incoming and one at the bound object
        IM_DEBUG_LOG("Trying to destroy window which is still referenced multiple times.");
    }
    #endif

    bound->renderTarget->setActive(false);

    gfx->getSceneManager()->dropPrimaryTarget(bound->renderTarget);
    gfx->getSurfaceManager()->destroySurface(_STD move(bound->surface));

    bound->sceneView.reset();

    /**/
    bound.reset();
    #ifdef _DEBUG
    if (wnd.use_count() > 1) {
        // We should just have one instance, which is incoming
        IM_DEBUG_LOG("Trying to destroy window which is still referenced multiple times.");
    }
    #endif
}

sptr<Window> WindowManager::requestWindow(
    cref<math::ivec2> size_,
    const wptr<Window> parent_,
    const non_owning_rptr<scene::IRenderScene> scene_
) {

    auto* const scene { scene_ == nullptr ? resolveScene(parent_.lock()) : scene_ };
    if (not scene) {
        IM_CORE_ERROR("Tried to create new window without providing any resolvable scene.");
        return nullptr;
    }

    /**/

    auto* const platform = Engine::getEngine()->getPlatform();
    const auto* const gfx = Engine::getEngine()->getGraphics();

    auto window = await(Future {
        platform->makeNativeWindow("New Window"sv, math::iExtent2D { size_.x, size_.y })
    });

    /**/

    auto surface = gfx->getSurfaceManager()->makeSurface(_STD move(window));
    surface->setup();

    const auto swapchain = make_sptr<gfx::VkSurfaceSwapchain>(surface);
    swapchain->setup(gfx->getCurrentDevice());

    surface->setSwapchain(swapchain);

    /**/

    auto sceneView { make_uptr<gfx::scene::SceneView>(nullptr, scene) };

    /**/

    auto target = make_sptr<gfx::RenderTarget>();
    target->use(gfx->getCurrentDevice());
    target->use(swapchain.get());
    target->use(surface);
    target->use(gfx->getRenderer(AssocKey<string>::from("UIRenderer")).get());

    /**/

    target->buildPasses(sceneView.get());

    gfx->getSceneManager()->addPrimaryTarget(target);

    /**/

    auto wnd { make_sptr<Window>() };

    _windows.push_back(make_uptr<BoundWindow>(
        target,
        _STD move(sceneView),
        surface,
        wnd
    ));

    /**/

    return wnd;
}
