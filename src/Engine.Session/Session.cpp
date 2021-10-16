#include "Session.hpp"

#include <Engine.Common/stdafx.h>
#include <Engine.Scheduler/Thread/Thread.hpp>

#include "Win32Window.hpp"

using namespace ember::engine;
using namespace ember;

Session::Session() {
    setup();
}

Session::~Session() {
    SessionModules::~SessionModules();
    tidy();
}

void Session::tidy() {

    for (auto entry : _windows) {
        entry->destroy();
    }

    _queue.finalize();
    if (_thread.joinable()) {
        _thread.join();
    }

    for (auto entry : _windows) {
        delete entry;
    }

    SDL_Quit();

    /**
     *
     */
    SessionModules::tidy();
}

void Session::setup() {

    /**
     *
     */
    SessionModules::setup();

    #ifdef _DEBUG
    auto code = SDL_Init(SDL_INIT_VIDEO);
    if (code != 0) {
        DEBUG_SNMSG(false, "SDL", "SDL should init correctly")
        DEBUG_SNMSG(false, "SDL", SDL_GetError())
        throw _STD runtime_error("Could not initialize SDL.");
    }
    #else
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        throw _STD runtime_error("Could not initialize SDL.");
    }
    #endif

    _thread = _STD thread {
        [&, this]() {

            SDL_Event event;
            _STD function<void()> fnc;

            while (!_queue.finalized()) {

                /**
                 * Poll events to stay responsive
                 */
                while (SDL_PollEvent(&event)) { }

                /**
                 * Check for signaled queue
                 */
                while (!_queue.empty()) {
                    _queue.waitPop(fnc);
                    fnc();
                }

                /**
                 * Sleep one cycle
                 */
                scheduler::thread::self::yield();
            }

            /**
             * Execute finalized queue
             */
            while (_queue.waitPop(fnc)) {
                fnc();
            }
        }
    };
}

cref<sptr<Session::value_type>> Session::get() noexcept {
    static sptr<Session> session = make_sptr<Session>();
    return session;
}

ptr<session::Window> Session::makeWindow(cref<string_view> title_, cref<math::uExtent2D> extent_) {
    #if ENV_MSVC
    auto* ptr = new session::Win32Window(&_queue, title_, extent_);
    _windows.push_back(ptr);
    return ptr;
    #else
    return nullptr;
    #endif
}

cref<GlobalEventEmitter> Session::emitter() const noexcept {
    return _emitter;
}

ref<GlobalEventEmitter> Session::emitter() noexcept {
    return _emitter;
}
