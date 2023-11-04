#include "WindowManager.hpp"

#include <Heliogrim/Inbuilt.hpp>
#include <Heliogrim/Future.hpp>
#include <Engine.Core/Engine.hpp>
#include <Engine.GFX/RenderTarget.hpp>
#include <Engine.GFX/Surface/Surface.hpp>
#include <Engine.Logging/Logger.hpp>
#include <Engine.Input/Input.hpp>

#include "BoundWindow.hpp"
#include "Window.hpp"
#include "Engine.GFX/Graphics.hpp"
#include "Engine.GFX.Scene/View/SceneView.hpp"
#include "Engine.GFX/Surface/SurfaceManager.hpp"
#include "Engine.GFX/Swapchain/VkSurfaceSwapchain.hpp"
#include "Engine.GFX.Scene/RenderSceneManager.hpp"
#include "Engine.Platform/Platform.hpp"

using namespace hg::engine::reflow;
using namespace hg;

sptr<WindowManager> WindowManager::_instance = nullptr;

cref<sptr<WindowManager::this_type>> WindowManager::get() {
    return _instance;
}

cref<sptr<WindowManager::this_type>> WindowManager::make() {
    if (!_instance) {
        _instance = sptr<WindowManager>(new WindowManager());
        _instance->setup();
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

void WindowManager::setup() {

    auto* const input { Engine::getEngine()->getInput() };
    auto& emitter { input->emitter() };

    /**/

    emitter.on<input::event::MouseButtonEvent>(
        [this](const auto& event_) {
            const auto wnd { resolveWindow(event_._pointer) };
            if (wnd) {
                dispatch(wnd, event_);
            }
        }
    );
    emitter.on<input::event::MouseMoveEvent>(
        [this](const auto& event_) {
            const auto prevWnd { resolveWindow(event_._pointer - event_._delta) };
            const auto nextWnd { resolveWindow(event_._pointer) };

            if (prevWnd && prevWnd != nextWnd) {
                dispatch(prevWnd, event_);
            }

            if (nextWnd) {
                dispatch(nextWnd, event_);
            }
        }
    );
    emitter.on<input::event::MouseWheelEvent>(
        [this](const auto& event_) {
            const auto wnd { resolveWindow(event_._pointer) };
            if (wnd) {
                dispatch(wnd, event_);
            }
        }
    );
    emitter.on<input::event::DragDropEvent>(
        [this](const auto& event_) {
            const auto wnd { resolveWindow(event_._pointer) };
            if (wnd) {
                dispatch(wnd, event_);
            }
        }
    );
    emitter.on<input::event::KeyboardEvent>(
        [this](const auto& event_) {

            sptr<Window> wnd { nullptr };
            for (const auto& entry : _focusOrder) {
                if (not entry.expired()) {
                    wnd = entry.lock();
                    break;
                }
            }

            if (wnd != nullptr) {
                dispatch(wnd, event_);
            }
        }
    );
}

void WindowManager::tidy() {

    _focusOrder.clear();

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

sptr<Window> WindowManager::resolveWindow(cref<math::ivec2> position_) const noexcept {

    const math::vec2 pos { static_cast<float>(position_.x), static_cast<float>(position_.y) };
    for (const auto& entry : _windows) {

        const auto* const wnd { entry->window.get() };
        if (intersects(wnd->layoutState().layoutOffset, wnd->layoutState().layoutSize, pos)) {
            return entry->window;
        }
    }

    return nullptr;
}

non_owning_rptr<engine::scene::IRenderScene> WindowManager::resolveScene(cref<sptr<Window>> window_) {

    const auto iter = _STD find_if(
        _windows.begin(),
        _windows.end(),
        [window_](cref<uptr<BoundWindow>> entry_) {
            return entry_.get()->window == window_;
        }
    );

    if (iter == _windows.end()) {
        return nullptr;
    }

    return iter->get()->sceneView->getScene();
}

void WindowManager::handleWindowResize(const ptr<BoundWindow> wnd_, cref<math::ivec2> nextSize_) const {

    const auto* const gfx = Engine::getEngine()->getGraphics();

    /**/

    const auto nextSwapchain = make_sptr<gfx::VkSurfaceSwapchain>(wnd_->surface);
    nextSwapchain->setup(gfx->getCurrentDevice());

    const auto prevSwapchain = wnd_->surface->swapchain();
    wnd_->surface->setSwapchain(nextSwapchain);

    /**/

    wnd_->renderTarget->rebuildPasses(nextSwapchain.get());
    wnd_->window->setClientSize(math::vec2 { nextSwapchain->extent() });

    /**/

    #ifdef _DEBUG
    if (prevSwapchain.use_count() > 1) {
        IM_DEBUG_LOG("Destroying swapchain which is still referenced multiple times.");
    }
    #endif

    prevSwapchain->destroy();
}

void WindowManager::destroyWindow(mref<sptr<Window>> window_) {

    const auto wnd { _STD move(window_) };

    const auto iter {
        _STD find_if(
            _windows.begin(),
            _windows.end(),
            [wnd](const auto& entry_) {
                return entry_.get()->window == wnd;
            }
        )
    };

    if (iter == _windows.end()) {
        // Shared pointer will destroy itself and we don't hold a reference
        return;
    }

    auto bound = _STD move(*iter);
    _windows.erase(iter);

    /**/

    bound->surface->getNativeWindow()->resizeEmitter().remove(bound->resizeListen);

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
    string_view title_,
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

    auto window = await(
        Future {
            platform->makeNativeWindow(title_, math::iExtent2D { size_.x, size_.y })
        }
    );

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
    target->use(gfx->getRenderer("UIRenderer").get());

    /**/

    target->buildPasses(sceneView.get());

    gfx->getSceneManager()->addPrimaryTarget(target);

    /**/

    auto wnd { make_sptr<Window>() };
    auto bound {
        make_uptr<BoundWindow>(
            target,
            _STD move(sceneView),
            surface,
            wnd,
            ~0
        )
    };

    /**/

    bound->resizeListen = surface->getNativeWindow()->resizeEmitter().on(
        [this, wnd = bound.get()](cref<engine::platform::PlatformResizeEvent> event_) {
            handleWindowResize(wnd, event_.getNextSize());
        }
    );

    wnd->setClientSize(math::vec2 { swapchain->extent() });

    /**/

    _windows.push_back(_STD move(bound));
    return wnd;
}

void WindowManager::interceptFocusEvent(cref<sptr<Window>> window_, cref<FocusEvent> event_) {

    if (not _focusOrder.empty()) {

        if (not _focusOrder.back().expired() && _focusOrder.back().lock() == window_) {
            return;
        }

        auto where = _STD ranges::remove_if(
            _focusOrder,
            [window_](const auto& entry_) {
                if (entry_.expired()) {
                    return true;
                }

                const auto wnd { entry_.lock() };
                return wnd == window_;
            }
        );

        _focusOrder.erase(where.begin(), where.end());
    }

    _focusOrder.push_back(window_);
}
