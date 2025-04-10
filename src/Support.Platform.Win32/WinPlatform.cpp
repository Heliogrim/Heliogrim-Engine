#include "WinPlatform.hpp"

#include <ole2.h>

#include <Engine.Asserts/Breakpoint.hpp>
#include <Engine.Common/SDL2.hpp>
#include <Engine.Common/stdafx.h>
#include <Engine.Common/Concurrent/Promise.hpp>
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
			const auto oleInitRes = OleInitialize(NULL);
			assert(oleInitRes == S_OK);

			// Loop every signal and dispatch events
			while (!_platformQueue.finalized()) {
				processInternal();
			}

			// While exiting, make sure queue is done
			std::function<void()> fnc { nullptr };
			while (_platformQueue.waitPop(fnc)) {
				fnc();
			}
		}
	};

}

void WinPlatform::tidy() {

	if (not _platformQueue.finalized()) {
		_platformQueue.finalize();
	}

	if (_thread.joinable()) {
		_thread.join();
	}

	/**/

	if (not _windows.empty()) {
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
	assert(oleInitRes == S_OK);

	/**/

	auto sdlInitRes = SDL_Init(SDL_INIT_VIDEO);
	if (sdlInitRes != 0) {
		IM_CORE_ERROR("SDL could not initialize correctly.");
		IM_CORE_ERROR(SDL_GetError());
		throw std::runtime_error("Could not initialize SDL.");
	}

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
					SDL_WINDOWPOS_CENTERED,
					SDL_WINDOWPOS_CENTERED,
					extent.width,
					extent.height,
					SDL_WINDOW_VULKAN | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_RESIZABLE
				),
				NativeWindowFlags { NativeWindowFlagBits::eMovable } | NativeWindowFlagBits::eResizable | NativeWindowFlagBits::eShown,
				math::ivec2 { extent.offsetX, extent.offsetY },
				math::ivec2 { extent.width, extent.height },
				title
			);

			::hg::assertrt(wnd && wnd->sdl());

			/**/

			SDL_SetWindowMinimumSize(wnd->sdl(), 128L, 128L);
			Engine::getEngine()->getInput()->captureWindow(wnd.get());
			_windows.push_back(wnd.get());

			/**/

			return wnd;
		}
	};

	const auto future { promise.get() };
	while (!_platformQueue.finalized() && not _platformQueue.try_push(std::move(promise))) {
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
	while (!_platformQueue.finalized() && not _platformQueue.try_push(std::move(promise))) {
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
			case SDL_EventType::SDL_QUIT: {
				// Warning: We need to call stop via scheduler, cause this thread will deadlock itself (recursive) due to window ownership and close behavior
				// Warning: Undefined behavior if called multiple times
				::hg::engine::scheduler::exec(
					[]() {
						Engine::getEngine()->getEmitter().emit(core::SignalShutdownEvent {});
					}
				);
				break;
			}
			case SDL_EventType::SDL_WINDOWEVENT: {
				// Warning: We need to call stop via scheduler
				// Warning: Temporary solution
				if (event.window.event == SDL_WINDOWEVENT_RESIZED) {

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

				if (event.window.event == SDL_WINDOWEVENT_CLOSE) {
					// TODO:
					break;
				}
			}
			case SDL_EventType::SDL_KEYUP:
			case SDL_EventType::SDL_KEYDOWN: {
				// Warning: Temporary solution

				if (event.key.keysym.sym == SDLK_UNKNOWN) {
					break;
				}

				Engine::getEngine()->getInput()->bufferEvent(
					input::event::KeyboardEvent::typeId,
					make_uptr<input::event::KeyboardEvent>(
						static_cast<char>(event.key.keysym.sym),
						event.type == SDL_EventType::SDL_KEYDOWN,
						event.key.keysym.mod
					)
				);
				break;
			}
			case SDL_EventType::SDL_MOUSEMOTION: {
				// Warning: Temporary solution

				math::ivec2 point { event.motion.x, event.motion.y };
				math::ivec2 delta { event.motion.xrel, event.motion.yrel };
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
			case SDL_EventType::SDL_MOUSEBUTTONDOWN:
			case SDL_EventType::SDL_MOUSEBUTTONUP: {
				// Warning: Temporary solution

				math::ivec2 point { event.button.x, event.button.y };
				const bool down { event.button.state == SDL_PRESSED };
				u32 button { event.button.button };

				Engine::getEngine()->getInput()->bufferEvent(
					input::event::MouseButtonEvent::typeId,
					make_uptr<input::event::MouseButtonEvent>(
						point,
						button,
						down,
						0uL
					)
				);
				break;
			}
			case SDL_EventType::SDL_MOUSEWHEEL: {
				// Warning: Temporary solution

				math::ivec2 point { -1, -1 };
				math::vec2 value { event.wheel.preciseX, event.wheel.preciseY };

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
			case SDL_EventType::SDL_DROPFILE: {
				// Warning: Temporary solution

				auto* rfile { event.drop.file };

				/*
				math::ivec2 point { lastPoint };
				SDL_PumpEvents();
				[[maybe_unused]] u32 _ { SDL_GetMouseState(&point.x, &point.y) };

				auto type { input::event::DragDropEventType::eDropFileType };
				void* data { rfile };

				const input::event::DragDropEvent dde { point, type, data };
				emitter().emit(dde);
				 */

				SDL_free(rfile);
			}
		}
	}

	/**
	 * Check for signaled queue
	 */
	std::function<void()> fnc { nullptr };
	while (!_platformQueue.empty()) {
		_platformQueue.waitPop(fnc);
		fnc();
	}

	/**
	 * Sleep one cycle
	 */
	scheduler::thread::self::yield();

}
