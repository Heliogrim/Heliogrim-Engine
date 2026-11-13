#include "WinPlatform.hpp"

#include <ole2.h>
#include <Engine.Asserts/Breakpoint.hpp>
#include <Engine.Common/SDL3.hpp>
#include <Engine.Common/stdafx.h>
#include <Engine.Core/Engine.hpp>
#include <Engine.Core/Event/SignalShutdownEvent.hpp>
#include <Engine.Event/GlobalEventEmitter.hpp>
#include <Engine.Graphics/Graphics.hpp>
#include <Engine.Input/Input.hpp>
#include <Engine.Input/KeyboardEvent.hpp>
#include <Engine.Input/MouseButtonEvent.hpp>
#include <Engine.Input/MouseMoveEvent.hpp>
#include <Engine.Input/MouseWheelEvent.hpp>
#include <Engine.Logging/Logger.hpp>
#include <Engine.Scheduler/Async.hpp>

#include "Win32Window.hpp"

using namespace hg::engine::scheduler;
using namespace hg::engine::platform;
using namespace hg::engine;
using namespace hg;

namespace {
	constexpr UINT WM_APP_DRAIN = WM_APP + 1u;
	constexpr auto drain_class_name = L"Heliogrim Platform Drain";

	LRESULT CALLBACK drainWindowProc(HWND hwnd_, UINT msg_, WPARAM wParam_, LPARAM lParam_);

	void nudgeDragCapture(const DWORD platformThreadId_);
}

WinPlatform::WinPlatform() :
	_platformThread("Platform Thread", {}, engine::scheduler::thread::priority::eHigh) {}

WinPlatform::~WinPlatform() = default;

void WinPlatform::setupThread() {

	_platformThread.start(
		[this](ref<async::AffineThread> self_) {

			// Each context which interacts with ole requires this init
			const auto oleInitRes = OleInitialize(nullptr);
			::hg::assertrt(oleInitRes == S_OK);

			const auto instance = GetModuleHandleW(nullptr);
			const WNDCLASSEXW windowClass {
				.cbSize = sizeof(WNDCLASSEXW),
				.lpfnWndProc = drainWindowProc,
				.hInstance = instance,
				.lpszClassName = drain_class_name
			};
			::hg::assertrt(RegisterClassExW(&windowClass) != 0);

			const auto hwnd = CreateWindowExW(0, drain_class_name, L"", 0, 0, 0, 0, 0, HWND_MESSAGE, nullptr, instance, nullptr);
			::hg::assertrt(hwnd != nullptr);

			self_.setWaker(
				[hwnd, threadId = GetCurrentThreadId()]() {
					PostMessageW(hwnd, WM_APP_DRAIN, 0, 0);
					nudgeDragCapture(threadId);
				}
			);

			::hg::assertrt(SDL_AddEventWatch(&WinPlatform::eventWatch, this));

			for (;;) {
				self_.drain();
				if (self_.stopping()) {
					self_.drain();
					break;
				}

				MsgWaitForMultipleObjectsEx(0, nullptr, INFINITE, QS_ALLINPUT, MWMO_INPUTAVAILABLE);

				// The event watch already translated everything; polling only pumps (and dispatches WM_APP_DRAIN).
				SDL_Event event;
				while (SDL_PollEvent(&event)) {}
			}

			SDL_RemoveEventWatch(&WinPlatform::eventWatch, this);
			// An in-flight stop() wake must not post to a destroyed (or reused) handle; an empty waker is a no-op.
			self_.setWaker({});
			DestroyWindow(hwnd);
			UnregisterClassW(drain_class_name, instance);
			OleUninitialize();
		}
	);

}

void WinPlatform::tidy() {

	_platformThread.stop();

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

ref<async::AffineThread> WinPlatform::platformThread() const noexcept {
	return _platformThread;
}

hg::Future<uptr<NativeWindow>> WinPlatform::makeNativeWindow(
	const string_view title_,
	cref<math::Extent2i32> extent_
) {
	// Copy the title, the view must not dangle until the platform thread runs.
	return _platformThread.post(
		[this, title = std::string { title_ }, extent = extent_]() -> uptr<NativeWindow> {

			/**/
			auto wnd = make_uptr<Win32Window>(
				SDL_CreateWindow(
					title.c_str(),
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

			// Register before the SDL calls below so their synchronous move/resize events find the window.
			// Events emitted inside SDL_CreateWindow itself still precede registration and are dropped (accepted).
			Engine::getEngine()->getInput()->captureWindow(wnd.get());
			_windows.push_back(wnd.get());

			SDL_SetWindowPosition(wnd->sdl(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
			SDL_SetWindowMinimumSize(wnd->sdl(), 128L, 128L);

			/**/

			return wnd;
		}
	);
}

hg::Future<bool> WinPlatform::destroyNativeWindow(
	mref<uptr<NativeWindow>> window_
) {
	return _platformThread.post(
		[this, window = std::move(window_)]() {

			/**/
			Engine::getEngine()->getInput()->releaseWindow(window.get());
			_windows.erase(std::remove(_windows.begin(), _windows.end(), window.get()));
			/**/

			const auto sdlWnd { static_cast<ptr<Win32Window>>(window.get())->sdl() };
			SDL_DestroyWindow(sdlWnd);

			return true;
		}
	);
}

bool WinPlatform::eventWatch(ptr<void> self_, ptr<SDL_Event> event_) {
	const auto self = static_cast<ptr<WinPlatform>>(self_);
	if (self->_platformThread.isCurrent()) {
		self->translateEvent(*event_);
		return true;
	}

	// SDL runs watches on the pushing thread (raw input, clipboard); `_windows` is platform-owned, so hop the POD events over.
	switch (event_->type) {
		case SDL_EVENT_QUIT:
		case SDL_EVENT_WINDOW_RESIZED:
		case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
		case SDL_EVENT_KEY_UP:
		case SDL_EVENT_KEY_DOWN:
		case SDL_EVENT_MOUSE_MOTION:
		case SDL_EVENT_MOUSE_BUTTON_DOWN:
		case SDL_EVENT_MOUSE_BUTTON_UP:
		case SDL_EVENT_MOUSE_WHEEL: {
			// Boxed: self + SDL_Event (128 bytes) exceeds the item's inline storage.
			self->_platformThread.postDetached([self, e = make_uptr<SDL_Event>(*event_)]() { self->translateEvent(*e); });
			break;
		}
		default: break;
	}
	return true;
}

void WinPlatform::translateEvent(ref<const SDL_Event> event_) {
	switch (event_.type) {
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
			auto* targetWnd = SDL_GetWindowFromID(event_.window.windowID);
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
				make_uptr<PlatformResizeEvent>(*iter, math::ivec2 { event_.window.data1, event_.window.data2 })
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

			if (event_.key.key == SDLK_UNKNOWN) {
				break;
			}

			Engine::getEngine()->getInput()->bufferEvent(
				input::event::KeyboardEvent::typeId,
				make_uptr<input::event::KeyboardEvent>(
					static_cast<char>(event_.key.key),
					event_.type == SDL_EventType::SDL_EVENT_KEY_DOWN,
					event_.key.mod
				)
			);
			break;
		}
		case SDL_EventType::SDL_EVENT_MOUSE_MOTION: {
			// Warning: Temporary solution

			math::fvec2 point { event_.motion.x, event_.motion.y };
			math::fvec2 delta { event_.motion.xrel, event_.motion.yrel };
			const auto button { event_.motion.state };

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

			const auto point = math::fvec2 { event_.button.x, event_.button.y };
			const auto button = event_.button.button;

			Engine::getEngine()->getInput()->bufferEvent(
				input::event::MouseButtonEvent::typeId,
				make_uptr<input::event::MouseButtonEvent>(
					point,
					button,
					event_.button.down,
					0uL
				)
			);
			break;
		}
		case SDL_EventType::SDL_EVENT_MOUSE_WHEEL: {
			// Warning: Temporary solution

			auto point = math::fvec2 { -1.f, -1.f };
			const auto value = math::vec2 { event_.wheel.mouse_x, event_.wheel.mouse_y };

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

/**/

namespace {
	LRESULT CALLBACK drainWindowProc(HWND hwnd_, UINT msg_, WPARAM wParam_, LPARAM lParam_) {
		// Also dispatched by modal loops and nested inside SDL_PollEvent, so posted work runs there too.
		if (msg_ == WM_APP_DRAIN) {
			async::AffineThread::current()->drain();
			return 0;
		}

		return DefWindowProcW(hwnd_, msg_, wParam_, lParam_);
	}

	// DoDragDrop only wakes on input, so poke its capture window with the current cursor (and button) state.
	// Looks the capture up via the platform thread's GUI info, so a stop() before the first QueryContinueDrag still nudges.
	void nudgeDragCapture(const DWORD platformThreadId_) {
		if (not dragActive().load(std::memory_order_acquire)) {
			return;
		}

		GUITHREADINFO info { .cbSize = sizeof(GUITHREADINFO) };
		if (not GetGUIThreadInfo(platformThreadId_, &info) || info.hwndCapture == nullptr) {
			return;
		}

		const auto capture = info.hwndCapture;

		POINT point {};
		if (not GetCursorPos(&point) || not ScreenToClient(capture, &point)) {
			return;
		}

		WPARAM keys = 0;
		const auto down = [](int key_) { return (GetAsyncKeyState(key_) & 0x8000) != 0; };
		// GetAsyncKeyState reports physical buttons; OLE compares against logical ones (GetKeyState in startDragDrop).
		const bool swapped = GetSystemMetrics(SM_SWAPBUTTON) != 0;
		if (down(VK_LBUTTON)) { keys |= swapped ? MK_RBUTTON : MK_LBUTTON; }
		if (down(VK_RBUTTON)) { keys |= swapped ? MK_LBUTTON : MK_RBUTTON; }
		if (down(VK_MBUTTON)) { keys |= MK_MBUTTON; }
		// Same bits as the drag's baseline in startDragDrop (no X buttons there), so the nudge never looks like a key change.
		if (down(VK_SHIFT)) { keys |= MK_SHIFT; }
		if (down(VK_CONTROL)) { keys |= MK_CONTROL; }

		PostMessageW(capture, WM_MOUSEMOVE, keys, MAKELPARAM(point.x, point.y));
	}
}
