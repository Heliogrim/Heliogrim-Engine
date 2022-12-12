#include "WinPlatform.hpp"

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

using namespace ember::engine::scheduler;
using namespace ember::engine::platform;
using namespace ember::engine;
using namespace ember;

WinPlatform::WinPlatform() = default;

WinPlatform::~WinPlatform() = default;

void WinPlatform::setupThread() {

    _thread = _STD thread {
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
            _STD function<void()> fnc { nullptr };
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
        throw _STD runtime_error("Could not initialize SDL.");
    }

    /**/

    setupThread();
}

ref<task::SignaledQueue> WinPlatform::platformQueue() const noexcept {
    return const_cast<ref<task::SignaledQueue>>(_platformQueue);
}

ember::concurrent::future<uptr<NativeWindow>> WinPlatform::makeNativeWindow(
    const string_view title_, cref<math::iExtent2D> extent_
) {

    ::ember::concurrent::promise<uptr<NativeWindow>> promise {
        [title = title_, extent = extent_] {

            /**/
            auto wnd = make_uptr<Win32Window>(
                SDL_CreateWindow(
                    title.data(),
                    SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                    extent.width, extent.height,
                    SDL_WINDOW_VULKAN | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_RESIZABLE
                ),
                NativeWindowFlags { NativeWindowFlagBits::eMovable } |
                NativeWindowFlagBits::eResizable |
                NativeWindowFlagBits::eShown,
                math::ivec2 { extent.offsetX, extent.offsetY },
                math::ivec2 { extent.width, extent.height },
                title
            );

            assert(wnd && wnd->sdl());

            /**/
            //Engine::getEngine()->getInput()->captureWindow(wnd);
            /**/

            return wnd;
        }
    };

    const auto future { promise.get() };
    while (!_platformQueue.finalized() && not _platformQueue.try_push(_STD move(promise))) {
        __noop();
    }

    return future;
}

ember::concurrent::future<bool> WinPlatform::destroyNativeWindow(
    mref<uptr<NativeWindow>> window_
) {

    _STD shared_ptr<NativeWindow> holder { _STD move(window_) };

    ::ember::concurrent::promise<bool> promise {
        [window = holder] {

            /**/
            //Engine::getEngine()->getInput()->releaseWindow(wnd);
            /**/

            const auto sdlWnd { static_cast<ptr<Win32Window>>(window.get())->sdl() };
            SDL_DestroyWindow(sdlWnd);

            return true;
        }
    };

    const auto future { promise.get() };
    while (!_platformQueue.finalized() && not _platformQueue.try_push(_STD move(promise))) {
        __noop();
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
                ::ember::engine::scheduler::exec([]() {
                    Engine::getEngine()->getEmitter().emit(core::SignalShutdownEvent {});
                });
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
                            Engine::getEngine()->getGraphics()->__tmp__resize(extent);
                        })
                    };

                    resizeTask->srcStage() = scheduler::ScheduleStageBarriers::eAll;
                    resizeTask->dstStage() = scheduler::ScheduleStageBarriers::eNetworkPushStrong;

                    scheduler::exec(_STD move(resizeTask));
                    break;
                }
            }
            case SDL_EventType::SDL_KEYDOWN: {
                // Warning: Temporary solution

                if (event.key.keysym.sym == SDLK_UNKNOWN) {
                    break;
                }

                const input::event::KeyboardEvent ke {
                    static_cast<char>(event.key.keysym.sym),
                    true,
                    event.key.keysym.mod
                };
                Engine::getEngine()->getEmitter().emit(ke);
                break;
            }
            case SDL_EventType::SDL_MOUSEMOTION: {
                // Warning: Temporary solution

                math::ivec2 point { event.motion.x, event.motion.y };
                math::ivec2 delta { event.motion.xrel, event.motion.yrel };
                const auto button { event.motion.state };

                const input::event::MouseMoveEvent mme { point, delta, button, 0ui32 };
                Engine::getEngine()->getEmitter().emit(mme);
                break;
            }
            case SDL_EventType::SDL_MOUSEBUTTONDOWN:
            case SDL_EventType::SDL_MOUSEBUTTONUP: {
                // Warning: Temporary solution

                math::ivec2 point { event.button.x, event.button.y };
                const bool down { event.button.state == SDL_PRESSED };
                u32 button { event.button.button };

                const input::event::MouseButtonEvent mbe { point, button, down, 0ui32 };
                Engine::getEngine()->getEmitter().emit(mbe);
                break;
            }
            case SDL_EventType::SDL_MOUSEWHEEL: {
                // Warning: Temporary solution

                math::ivec2 point { -1, -1 };
                math::vec2 value { event.wheel.preciseX, event.wheel.preciseY };

                const input::event::MouseWheelEvent mwe { point, value };
                Engine::getEngine()->getEmitter().emit(mwe);
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
    _STD function<void()> fnc { nullptr };
    while (!_platformQueue.empty()) {
        _platformQueue.waitPop(fnc);
        fnc();
    }

    /**
     * Sleep one cycle
     */
    scheduler::thread::self::yield();

}
