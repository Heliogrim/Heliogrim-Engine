#include "Input.hpp"

#include <Engine.Session/Win32Window.hpp>

#include "DragDrop/Win32DragDropReceiver.hpp"
#include "DragDrop/Win32DragDropSender.hpp"
#include <Engine.Scheduler/Async.hpp>

using namespace ember::engine::input;
using namespace ember::engine;
using namespace ember;

Input::Input(cref<sptr<Session>> session_) noexcept :
    _session(session_),
    _dragDropReceiver(nullptr),
    _dragDropSender(nullptr) {}

Input::~Input() {
    tidy();
}

void Input::setup() {

    _dragDropSender = new Win32DragDropSender();
    _dragDropSender->setup();

}

void Input::tidy() {

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

sptr<Session> Input::session() const noexcept {
    return _session;
}

const ptr<input::DragDropSender> Input::dragDropSender() const noexcept {
    return _dragDropSender;
}

void Input::captureWindow(const ptr<session::Window> window_) {

    assert(not _dragDropReceiver);

    auto* sdlWnd = static_cast<const ptr<session::Win32Window>>(window_)->_wnd;
    SDL_SysWMinfo info {};
    SDL_VERSION(&info.version);
    SDL_GetWindowWMInfo(sdlWnd, &info);

    const HWND hwnd = info.info.win.window;

    _dragDropReceiver = new Win32DragDropReceiver(hwnd);
    _dragDropReceiver->setup();

    /**/

    _dragDropReceiver->setOnDrop([session = _session](sptr<event::DragDropEvent> event_) {

        // Attention: This function callback is actual external thread context
        // Attention: Not allowed to use default/fiber control flow

        scheduler::exec([event = _STD move(event_), session]() {
            // Dispatch event with fiber context
            session->emitter().emit(*event);
        });

        return false;
    });
}

void Input::releaseWindow(const ptr<session::Window> window_) {

    if (_dragDropReceiver) {
        _dragDropReceiver->destroy();

        delete _dragDropReceiver;
        _dragDropReceiver = nullptr;
    }

}
