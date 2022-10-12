#include "Session.hpp"

#include <Ember/Ember.hpp>
#include <Engine.Common/stdafx.h>
#include <Engine.Common/Concurrent/Promise.hpp>
#include <Engine.Event/ShutdownEvent.hpp>
#include <Engine.Scheduler/Scheduler.hpp>
#include <Engine.Scheduler/Thread/Thread.hpp>

#if true
#include <Engine.GFX/Graphics.hpp>
#include <Engine.Input/MouseButtonEvent.hpp>
#include <Engine.Input/MouseMoveEvent.hpp>
#endif

#include "Win32Window.hpp"

using namespace ember::engine;
using namespace ember;

Session::Session() = default;

Session::~Session() {
    tidy();
}

void Session::tidy() {

    /**
     *
     */
    _moduleManager.tidy();

    /**
     *
     */
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
}

void Session::setup() {

    /**
     *
     */
    _moduleManager.setup();

    /**
     *
     */
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

            #ifdef WIN32
            SetThreadDescription(GetCurrentThread(), L"Session Thread");
            #endif

            SDL_Event event;
            _STD function<void()> fnc;

            while (!_queue.finalized()) {

                /**
                 * Poll events to stay responsive
                 */
                while (SDL_PollEvent(&event)) {
                    // TODO: Replace
                    switch (event.type) {
                        case SDL_EventType::SDL_KEYDOWN: {
                            if (event.key.keysym.sym != SDLK_ESCAPE) {
                                break;
                            }

                            // Warning: We need to call stop via scheduler, cause this thread will deadlock itself (recursive) due to window ownership and close behavior
                            // Warning: Undefined behavior if called multiple times
                            Scheduler::get().exec(scheduler::task::make_task(Session::stop));
                            break;
                        }
                        case SDL_EventType::SDL_QUIT: {
                            // Warning: We need to call stop via scheduler, cause this thread will deadlock itself (recursive) due to window ownership and close behavior
                            // Warning: Undefined behavior if called multiple times
                            Scheduler::get().exec(scheduler::task::make_task(Session::stop));
                            break;
                        }
                        case SDL_EventType::SDL_WINDOWEVENT: {
                            // Warning: We need to call stop via scheduler
                            // Warning: Temporary solution
                            if (event.window.event == SDL_WINDOWEVENT_RESIZED) {

                                math::uivec2 extent {
                                    static_cast<u32>(event.window.data1),
                                    static_cast<u32>(event.window.data2)
                                };
                                auto resizeTask {
                                    scheduler::task::make_task([extent]() {
                                        Session::get()->modules().graphics()->__tmp__resize(extent);
                                    })
                                };

                                resizeTask->srcStage() = scheduler::ScheduleStageBarriers::eAll;
                                resizeTask->dstStage() = scheduler::ScheduleStageBarriers::eNetworkPushStrong;

                                Scheduler::get().exec(_STD move(resizeTask));
                                break;
                            }
                        }
                        case SDL_EventType::SDL_MOUSEMOTION: {
                            // Warning: Temporary solution

                            math::ivec2 point { event.motion.x, event.motion.y };
                            math::ivec2 delta { event.motion.xrel, event.motion.yrel };

                            const input::event::MouseMoveEvent mme { point, delta, 0ui32, 0ui32 };
                            emitter().emit(mme);
                            break;
                        }
                        case SDL_EventType::SDL_MOUSEBUTTONDOWN:
                        case SDL_EventType::SDL_MOUSEBUTTONUP: {
                            // Warning: Temporary solution

                            math::ivec2 point { event.button.x, event.button.y };
                            const bool down { event.button.state == SDL_PRESSED };
                            u32 button { event.button.button };

                            const input::event::MouseButtonEvent mbe { point, button, down, 0ui32 };
                            emitter().emit(mbe);
                            break;
                        }
                    }
                }

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

void Session::start() {

    /**
     *
     */
    _moduleManager.start();

    auto p {
        ember::concurrent::promise<void>([core = &_core]() {
            /**
             * Invoke core function
             *
             * @see Core::start()
             */
            core->start();
        })
    };

    auto f = p.get();

    /**
     * Start Game Core
     */
    _moduleManager.scheduler()->exec(scheduler::task::make_task(
        p,
        TaskMask::eCritical,
        scheduler::ScheduleStageBarriers::eBottomWeak,
        scheduler::ScheduleStageBarriers::eBottomStrong
    ));

    /**
     * Await Game Core started
     */
    f.get();
    // await(f.signal());
}

void Session::stop() {

    auto session = engine::Session::get();

    /**
     *
     */
    session->emitter().emit<ShutdownEvent>();

    /**
     *
     */
    auto p {
        ember::concurrent::promise<void>([core = &(session->_core)]() {
            /**
             * Invoke core function
             *
             * @see Core::stop()
             */
            core->stop();
        })
    };

    auto f = p.get();

    /**
     * Stop Game Core
     */
    session->_moduleManager.scheduler()->exec(scheduler::task::make_task(
        p,
        TaskMask::eCritical,
        scheduler::ScheduleStageBarriers::eAll,
        scheduler::ScheduleStageBarriers::eUndefined
    ));

    /**
     * Await Game Core stopped
     */
    await(f.signal());

    /**
     *
     */
    session->_moduleManager.stop();
}

void Session::wait() {
    _moduleManager.wait();
}

cref<session::EmberModuleManager> Session::modules() const noexcept {
    return _moduleManager;
}

cref<Core> Session::core() const noexcept {
    return _core;
}

cref<sptr<Session::value_type>> Session::get() noexcept {
    static sptr<Session> session = nullptr;

    if (!session) {
        auto ns { make_sptr<Session>() };
        session.swap(ns);
        session->setup();
    }

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
