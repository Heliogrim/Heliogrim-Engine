#include "WinPlatform.hpp"

#include <ole2.h>

#include <Engine.Asserts/Breakpoint.hpp>
#include <Engine.Async/Await/Promise.hpp>
#include <Engine.Common/SDL3.hpp>
#include <Engine.Common/stdafx.h>
#include <Engine.Core/Engine.hpp>
#include <Engine.Core/Event/SignalShutdownEvent.hpp>
#include <Engine.Event/GlobalEventEmitter.hpp>
#include <Engine.GFX/Graphics.hpp>
#include <Engine.Input/KeyboardEvent.hpp>
#include <Engine.Input/MouseButtonEvent.hpp>
#include <Engine.Input/MouseMoveEvent.hpp>
#include <Engine.Input/MouseWheelEvent.hpp>
#include <Engine.Logging/Logger.hpp>
#include <Engine.Scheduler/Async.hpp>

#include "Win32Window.hpp"
#include "Engine.Input/Input.hpp"

using namespace hg::engine::scheduler;
using namespace hg::engine::platform;
using namespace hg::engine;
using namespace hg;

WinPlatform::WinPlatform() = default;

WinPlatform::~WinPlatform() = default;

void WinPlatform::setupThread() {

	_thread = std::thread {
		[&, this] {

			SetThreadDescription(GetCurrentThread(), L"Platform Thread");

			// Each context which interacts with ole requires this init
			const auto oleInitRes = OleInitialize(nullptr);
			::hg::assertrt(oleInitRes == S_OK);

			// Loop every signal and dispatch events
			while (!_platformQueue.finalized()) {
				processInternal();
			}

			// While exiting, make sure queue is done
			std::function < void() > fnc { nullptr };
			while (_platformQueue.waitPop(fnc)) {
				fnc();
			}
		}
	};

}

void WinPlatform::tidy() {

	if (not
		_platformQueue.finalized()
	) {
		_platformQueue.finalize();
	}

	if (_thread.joinable()) {
		_thread.join();
	}

	/**/

	if (not
		_windows.empty()
	) {
		IM_CORE_ERROR("Tidy up platform interface failed due to undestructed windows.");
		#ifdef _DEBUG
		::hg::breakpoint();
		#endif
	}

	/**/

	SDL_Quit();
}

void WinPlatform::setup() {

	const auto oleInitRes = OleInitialize(NULL);
	::hg::assertrt(oleInitRes == S_OK);
	::hg::assertd(not SDL_WasInit(SDL_INIT_VIDEO));

	/**/

	const auto sdlInitSuccess = SDL_InitSubSystem(SDL_INIT_VIDEO);
	if (not sdlInitSuccess) {
		IM_CORE_ERROR("SDL could not initialize correctly.");
		IM_CORE_ERROR(SDL_GetError());
		throw std::runtime_error("Could not initialize SDL.");
	}

	// Attention: Configure SDL3 events to prevent registration of drag-drop handler
	SDL_SetEventEnabled(SDL_EVENT_DROP_FILE, false);
	SDL_SetEventEnabled(SDL_EVENT_DROP_TEXT, false);

	/**/

	setupThread();
}

ref<task::SignaledQueue> WinPlatform::platformQueue() const noexcept {
	return const_cast<ref<task::SignaledQueue>>(_platformQueue);
}

hg::concurrent::future<uptr<NativeWindow>> WinPlatform::makeNativeWindow(
	const string_view title_,
	cref<math::iExtent2D> extent_
) {

	::hg::concurrent::promise<uptr<NativeWindow>> promise {
		[this, title = title_, extent = extent_] {

			/**/
			auto wnd = make_uptr<Win32Window>(
				SDL_CreateWindow(
					title.data(),
					extent.width,
					extent.height,
					SDL_WINDOW_VULKAN | SDL_WINDOW_HIGH_PIXEL_DENSITY | SDL_WINDOW_RESIZABLE
				),
				NativeWindowFlags { NativeWindowFlagBits::eMovable } | NativeWindowFlagBits::eResizable | NativeWindowFlagBits::eShown,
				math::ivec2 { extent.offsetX, extent.offsetY },
				math::ivec2 { extent.width, extent.height },
				title
			);

			::hg::assertrt(wnd && wnd->sdl());

			/**/

			SDL_SetWindowPosition(wnd->sdl(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
			SDL_SetWindowMinimumSize(wnd->sdl(), 128L, 128L);
			Engine::getEngine()->getInput()->captureWindow(wnd.get());
			_windows.push_back(wnd.get());

			/**/

			return wnd;
		}
	};

	const auto future { promise.get() };
	while (!_platformQueue.finalized() && not
		_platformQueue.try_push(std::move(promise))
	) {
		// __noop();
	}

	return future;
}

hg::concurrent::future<bool> WinPlatform::destroyNativeWindow(
	mref<uptr<NativeWindow>> window_
) {

	std::shared_ptr<NativeWindow> holder { std::move(window_) };

	::hg::concurrent::promise<bool> promise {
		[this, window = holder] {

			/**/
			Engine::getEngine()->getInput()->releaseWindow(window.get());
			_windows.erase(std::remove(_windows.begin(), _windows.end(), window.get()));
			/**/

			const auto sdlWnd { static_cast<ptr<Win32Window>>(window.get())->sdl() };
			SDL_DestroyWindow(sdlWnd);

			return true;
		}
	};

	const auto future { promise.get() };
	while (!_platformQueue.finalized() && not
		_platformQueue.try_push(std::move(promise))
	) {
		// __noop();
	}

	return future;
}

void WinPlatform::processInternal() {
	// Poll events to stay responsive

	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		// TODO: Replace
		switch (event.type) {
			case SDL_EventType::SDL_EVENT_QUIT: {
				// Warning: We need to call stop via scheduler, cause this thread will deadlock itself (recursive) due to window ownership and close behavior
				// Warning: Undefined behavior if called multiple times
				::hg::engine::scheduler::exec(
					[]() {
						Engine::getEngine()->getEmitter().emit(core::SignalShutdownEvent {});
					}
				);
				break;
			}
			case SDL_EventType::SDL_EVENT_WINDOW_RESIZED: {
				// Warning: We need to call stop via scheduler
				// Warning: Temporary solution
				auto* targetWnd = SDL_GetWindowFromID(event.window.windowID);
				const auto iter {
					std::find_if(
						_windows.begin(),
						_windows.end(),
						[targetWnd](const auto& entry_) {
							return static_cast<ptr<Win32Window>>(entry_)->sdl() == targetWnd;
						}
					)
				};

				if (iter == _windows.end()) {
					break;
				}

				Engine::getEngine()->getInput()->bufferEvent(
					PlatformResizeEvent::typeId,
					make_uptr<PlatformResizeEvent>(*iter, math::ivec2 { event.window.data1, event.window.data2 })
				);
				break;
			}
			case SDL_EventType::SDL_EVENT_WINDOW_CLOSE_REQUESTED: {
				// TODO:
				break;
			}
			case SDL_EventType::SDL_EVENT_KEY_UP:
			case SDL_EventType::SDL_EVENT_KEY_DOWN: {
				// Warning: Temporary solution

				if (event.key.key == SDLK_UNKNOWN) {
					break;
				}

				Engine::getEngine()->getInput()->bufferEvent(
					input::event::KeyboardEvent::typeId,
					make_uptr<input::event::KeyboardEvent>(
						static_cast<char>(event.key.key),
						event.type == SDL_EventType::SDL_EVENT_KEY_DOWN,
						event.key.mod
					)
				);
				break;
			}
			case SDL_EventType::SDL_EVENT_MOUSE_MOTION: {
				// Warning: Temporary solution

				math::fvec2 point { event.motion.x, event.motion.y };
				math::fvec2 delta { event.motion.xrel, event.motion.yrel };
				const auto button { event.motion.state };

				Engine::getEngine()->getInput()->bufferEvent(
					input::event::MouseMoveEvent::typeId,
					make_uptr<input::event::MouseMoveEvent>(
						point,
						delta,
						button,
						0uL
					)
				);
				break;
			}
			case SDL_EventType::SDL_EVENT_MOUSE_BUTTON_DOWN:
			case SDL_EventType::SDL_EVENT_MOUSE_BUTTON_UP: {
				// Warning: Temporary solution

				const auto point = math::fvec2 { event.button.x, event.button.y };
				const auto button = event.button.button;

				Engine::getEngine()->getInput()->bufferEvent(
					input::event::MouseButtonEvent::typeId,
					make_uptr<input::event::MouseButtonEvent>(
						point,
						button,
						event.button.down,
						0uL
					)
				);
				break;
			}
			case SDL_EventType::SDL_EVENT_MOUSE_WHEEL: {
				// Warning: Temporary solution

				auto point = math::fvec2 { -1.f, -1.f };
				const auto value = math::vec2 { event.wheel.mouse_x, event.wheel.mouse_y };

				SDL_GetMouseState(&point.x, &point.y);

				Engine::getEngine()->getInput()->bufferEvent(
					input::event::MouseWheelEvent::typeId,
					make_uptr<input::event::MouseWheelEvent>(
						point,
						value
					)
				);
				break;
			}
		}
	}

	/**
	 * Check for signaled queue
	 */
	std::function < void() > fnc { nullptr };
	while (!_platformQueue.empty()) {
		_platformQueue.waitPop(fnc);
		fnc();
	}

	/**
	 * Sleep one cycle
	 */
	scheduler::thread::self::yield();

}