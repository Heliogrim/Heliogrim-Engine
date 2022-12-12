#include "Input.hpp"

#include <Engine.Core/Engine.hpp>
#include <Engine.Event/GlobalEventEmitter.hpp>
#include <Engine.Platform/Windows/Win32Window.hpp>
#include <Engine.Scheduler/Async.hpp>

#include "DragDrop/Win32DragDropReceiver.hpp"
#include "DragDrop/Win32DragDropSender.hpp"

using namespace ember::engine::input;
using namespace ember::engine;
using namespace ember;

Input::Input(const non_owning_rptr<Engine> engine_) noexcept :
    CoreModule(engine_),
    _dragDropReceiver(nullptr),
    _dragDropSender(nullptr) {}

Input::~Input() {
    Input::destroy();
}

void Input::setup() {
    _dragDropSender = new Win32DragDropSender();
    _dragDropSender->setup();
}

void Input::schedule() {}

void Input::desync() {}

void Input::destroy() {
    if (_dragDropReceiver) {
        _dragDropReceiver->destroy();

        delete _dragDropReceiver;
        _dragDropReceiver = nullptr;
    }

    if (_dragDropSender) {
        _dragDropSender->destroy();

        delete _dragDropSender;
        _dragDropSender = nullptr;
    }
}

const ptr<input::DragDropSender> Input::dragDropSender() const noexcept {
    return _dragDropSender;
}

void Input::captureWindow(const non_owning_rptr<platform::NativeWindow> nativeWindow_) {
    assert(not _dragDropReceiver);

    #ifdef WIN32
    auto* sdlWnd = static_cast<ptr<platform::Win32Window>>(nativeWindow_)->sdl();
    SDL_SysWMinfo info {};
    SDL_VERSION(&info.version);
    SDL_GetWindowWMInfo(sdlWnd, &info);

    const HWND hwnd = info.info.win.window;

    _dragDropReceiver = new Win32DragDropReceiver(hwnd);
    _dragDropReceiver->setup();

    /**/

    _dragDropReceiver->setOnDrop([engine = _engine](sptr<event::DragDropEvent> event_) {
        // Attention: This function callback is actual external thread context
        // Attention: Not allowed to use default/fiber control flow

        scheduler::exec([event = _STD move(event_), engine]() {
            // Dispatch event with fiber context
            engine->getEmitter().emit(*event);
        });

        return false;
    });
    #else
    #endif
}

void Input::releaseWindow(const non_owning_rptr<platform::NativeWindow> nativeWindow_) {
    if (_dragDropReceiver) {
        _dragDropReceiver->destroy();

        delete _dragDropReceiver;
        _dragDropReceiver = nullptr;
    }
}
