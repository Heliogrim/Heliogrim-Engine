#include "WindowManager.hpp"

#include <algorithm>
#include <ranges>
#include <Engine.Core/Engine.hpp>
#include <Engine.GFX/Surface/Surface.hpp>
#include <Engine.GFX/Surface/SurfaceManager.hpp>
#include <Engine.GFX/Swapchain/VkSurfaceSwapchain.hpp>
#include <Engine.GFX.Scene/RenderSceneManager.hpp>
#include <Engine.GFX/Graphics.hpp>
#include <Engine.GFX/RenderTarget.hpp>
#include <Engine.GFX.Scene/View/SceneView.hpp>
#include <Engine.Input/Input.hpp>
#include <Engine.Logging/Logger.hpp>
#include <Engine.Pedantic/Clone/Clone.hpp>
#include <Engine.Platform/Platform.hpp>
#include <Engine.Render.Scene/RenderSceneSystem.hpp>
#include <Engine.Scene/SceneBase.hpp>
#include <Heliogrim/Async/Await.hpp>
#include <Heliogrim/Async/Future.hpp>

#include "BoundWindow.hpp"
#include "Window.hpp"

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

	const auto input = Engine::getEngine()->getInput();
	auto& emitter = input->emitter();

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
		destroyWindow(std::move(entry));
	}

}

sptr<Window> WindowManager::resolveWindow(cref<math::ivec2> position_) const noexcept {

	const math::vec2 pos { static_cast<float>(position_.x), static_cast<float>(position_.y) };
	for (const auto& entry : _windows) {

		const auto* const wnd { entry->window.get() };
		if (intersects(wnd->getLayoutState().layoutOffset, wnd->getLayoutState().layoutSize, pos)) {
			return entry->window;
		}
	}

	return nullptr;
}

nmpt<engine::render::RenderSceneSystem> WindowManager::resolveRenderSystem(cref<sptr<Window>> window_) {

	const auto iter = std::ranges::find_if(
		_windows,
		[window_](cref<uptr<BoundWindow>> entry_) {
			return entry_->window == window_;
		}
	);

	if (iter == _windows.end()) {
		return nullptr;
	}

	return iter->get()->renderTarget->getSceneView()->getRenderSceneSystem();
}

void WindowManager::handleWindowResize(const ptr<BoundWindow> wnd_, cref<math::ivec2> nextSize_) const {

	if (
		math::ivec2 { wnd_->window->getClientSize() } == nextSize_ &&
		math::ivec2 { wnd_->surface->swapchain()->extent() } == nextSize_
	) {
		return;
	}

	/**/

	const auto gfx = Engine::getEngine()->getGraphics();

	/**/

	auto nextSwapChain = make_smr<gfx::VkSurfaceSwapchain>(wnd_->surface);
	nextSwapChain->setup(gfx->getCurrentDevice());

	auto prevSwapChain = wnd_->surface->swapchain();
	wnd_->surface->setSwapchain(clone(nextSwapChain));

	const auto nextWindowExtent = nextSwapChain->extent();

	/**/

	gfx->getSceneManager()->transitionToTarget(std::move(prevSwapChain), std::move(nextSwapChain), wnd_->surface);
	wnd_->window->setClientSize(math::vec2 { nextWindowExtent });
}

void WindowManager::destroyWindow(mref<sptr<Window>> window_) {

	const auto wnd { std::move(window_) };

	const auto iter {
		std::find_if(
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

	auto bound = std::move(*iter);
	_windows.erase(iter);

	/**/

	bound->surface->getNativeWindow()->resizeEmitter().remove(bound->resizeListen);

	/**/

	const auto platform = Engine::getEngine()->getPlatform();
	const auto gfx = Engine::getEngine()->getGraphics();

	#ifdef _DEBUG
	if (wnd.use_count() > 2) {
		// We have 2 references, one is incoming and one at the bound object
		IM_DEBUG_LOG("Trying to destroy window which is still referenced multiple times.");
	}
	#endif

	bound->renderTarget->setActive(false);
	gfx->getSceneManager()->dropPrimaryTarget(bound->renderTarget);

	bound->renderTarget.reset();
	gfx->getSurfaceManager()->destroySurface(std::move(bound->surface));

	bound.reset();

	/**/
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
	string_view renderer_,
	nmpt<scene::SceneBase> renderableScene_
) {

	nmpt<engine::render::RenderSceneSystem> renderSystem = nullptr;

	if (renderableScene_ != nullptr) {
		renderSystem = renderableScene_->getSceneSystem<render::RenderSceneSystem>();

	} else {
		renderSystem = resolveRenderSystem(parent_.lock());
	}

	if (renderSystem == nullptr) {
		IM_CORE_ERROR("Tried to create new window without providing any resolvable scene.");
		return nullptr;
	}

	/**/

	const auto platform = Engine::getEngine()->getPlatform();
	const auto gfx = Engine::getEngine()->getGraphics();

	auto window = await(
		Future {
			platform->makeNativeWindow(title_, math::iExtent2D { size_.x, size_.y })
		}
	);
	::hg::assertrt(window != nullptr);

	/**/

	auto surface = gfx->getSurfaceManager()->makeSurface(std::move(window));
	surface->setup();

	auto swapchain = make_smr<gfx::VkSurfaceSwapchain>(surface);
	swapchain->setup(gfx->getCurrentDevice());

	surface->setSwapchain(clone(swapchain));
	const auto surfaceExtent = swapchain->extent();

	/**/

	auto device = gfx->getCurrentDevice();
	auto renderer = gfx->getRenderer(renderer_, std::nothrow);

	if (renderer.empty()) {

		swapchain->destroy();
		surface->destroy();
		await(Future { platform->destroyNativeWindow(std::move(window)) });

		IM_CORE_ERROR("Requested new window with invalid renderer.");
		return nullptr;
	}

	auto sceneView = make_smr<gfx::scene::SceneView>(nullptr, renderSystem);

	/**/

	auto renderTarget = make_smr<gfx::RenderTarget>(std::move(device), renderer.get());

	[[maybe_unused]] auto sceneViewTransition = renderTarget->transitionToSceneView(std::move(sceneView));
	[[maybe_unused]] auto targetTransition = renderTarget->transitionToTarget(std::move(swapchain), surface);

	renderTarget->setup();
	renderTarget->setActive(true);

	/**/

	gfx->getSceneManager()->addPrimaryTarget(clone(renderTarget));

	/**/

	auto wnd { make_sptr<Window>() };
	auto bound {
		make_uptr<BoundWindow>(
			std::move(renderTarget),
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

	wnd->setClientSize(math::vec2 { surfaceExtent });

	/**/

	_windows.emplace_back(std::move(bound));
	return wnd;
}

void WindowManager::interceptFocusEvent(cref<sptr<Window>> window_, cref<FocusEvent> event_) {

	if (not _focusOrder.empty()) {

		if (not _focusOrder.back().expired() && _focusOrder.back().lock() == window_) {
			return;
		}

		auto where = std::ranges::remove_if(
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
