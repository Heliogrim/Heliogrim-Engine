#include "Input.hpp"

#include <Engine.Common/SDL2.hpp>
#include <Engine.Core/Engine.hpp>
#include <Engine.Event/GlobalEventEmitter.hpp>
#include <Engine.Input.Schedule/InputPipeline.hpp>
#include <Engine.Platform/Windows/Win32Window.hpp>
#include <Engine.Scheduler/Async.hpp>
#include <Engine.Scheduler/Fiber/Fiber.hpp>
#include <Engine.Scheduler/Pipeline/CompositePipeline.hpp>

#include "DragDropEvent.hpp"
#include "KeyboardEvent.hpp"
#include "MouseButtonEvent.hpp"
#include "MouseMoveEvent.hpp"
#include "MouseWheelEvent.hpp"
#include "DragDrop/Win32DragDropReceiver.hpp"
#include "DragDrop/Win32DragDropSender.hpp"

using namespace hg::engine::input;
using namespace hg::engine;
using namespace hg;

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

    /**
     * Scheduling Pipelines
     */
    auto inputPipeline = make_uptr<schedule::InputPipeline>();
    _engine->getScheduler()->getCompositePipeline()->addPipeline(_STD move(inputPipeline));
}

void Input::start() {}

void Input::stop() {}

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

void Input::tick() {

    _STD unique_lock<_STD mutex> lck { _bufferMtx };

    for (const auto& entry : _buffered) {

        switch (entry.first.data) {
            case event::MouseMoveEvent::typeId.data: {
                _emitter.emit<event::MouseMoveEvent>(static_cast<ptr<event::MouseMoveEvent>>(entry.second.get()));
                break;
            }
            case event::MouseWheelEvent::typeId.data: {
                _emitter.emit<event::MouseWheelEvent>(static_cast<ptr<event::MouseWheelEvent>>(entry.second.get()));
                break;
            }
            case event::MouseButtonEvent::typeId.data: {
                _emitter.emit<event::MouseButtonEvent>(static_cast<ptr<event::MouseButtonEvent>>(entry.second.get()));
                break;
            }
            case event::DragDropEvent::typeId.data: {
                _emitter.emit<event::DragDropEvent>(static_cast<ptr<event::DragDropEvent>>(entry.second.get()));
                break;
            }
            case event::KeyboardEvent::typeId.data: {
                _emitter.emit<event::KeyboardEvent>(static_cast<ptr<event::KeyboardEvent>>(entry.second.get()));
                break;
            }
            case platform::PlatformResizeEvent::typeId.data: {

                auto* const event = static_cast<
                    const ptr<platform::PlatformResizeEvent>>(entry.second.get());
                event->getWindow()->resizeEmitter().emit(event);

                _emitter.emit<platform::PlatformResizeEvent>(
                    *event
                );
            }
            default: {}
        }

    }

    _buffered.clear();
}

cref<GlobalEventEmitter> Input::emitter() const noexcept {
    return _emitter;
}

ref<GlobalEventEmitter> Input::emitter() noexcept {
    return _emitter;
}

bool Input::tryBufferEvent(const event_type_id eventType_, mref<uptr<Event>> event_) {

    _STD unique_lock<_STD mutex> lck { _bufferMtx, _STD defer_lock };
    if (not lck.try_lock()) {
        return false;
    }

    _buffered.emplace_back(eventType_, _STD move(event_));
    return true;
}

void Input::bufferEvent(const event_type_id eventType_, mref<uptr<Event>> event_) {
    _STD unique_lock<_STD mutex> lck { _bufferMtx };
    _buffered.emplace_back(eventType_, _STD move(event_));
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

    _dragDropReceiver->setOnDrop(
        [this](mref<uptr<event::DragDropEvent>> event_) {
            // Attention: This function callback is actual external thread context
            // Attention: Not allowed to use default/fiber control flow
            bufferEvent(event::DragDropEvent::typeId, _STD move(event_));
            return false;
        }
    );
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
