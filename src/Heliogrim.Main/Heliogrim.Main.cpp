#include "Heliogrim.Main.hpp"

#ifdef _PROFILING
#include <Engine.Common/Profiling/Profiler.hpp>
#include <Engine.Common/Profiling/Stopwatch.hpp>
#endif

#include <Heliogrim/Heliogrim.hpp>
#include <Heliogrim/HeliogrimStatic.hpp>

#include <Engine.Core/Event/SignalShutdownEvent.hpp>
#include <Engine.Core.Game/GameEngine.hpp>
#include <Engine.Event/GlobalEventEmitter.hpp>

using namespace hg;

using SignalShutdownEvent = ::hg::engine::core::SignalShutdownEvent;

int main() {
    #ifdef _PROFILING
    profiling::Profiler::make().startSession("main");
    SCOPED_STOPWATCH_V(__main__stopwatch)
    #endif

    #if USE_MAIN_THREAD_INJECTION
    beforeRoutine();
    #endif

    /**
     * Pre-Configure Engine Startup
     */
    if (not HeliogrimStatic::hasInitializer()) {
        HeliogrimStatic::useEngine<engine::GameEngine>();
        HeliogrimStatic::useSessionGetter<engine::GameEngine>(&engine::GameEngine::getGameSession);
    }

    /**
     * Instantiate Engine
     */
    Heliogrim::instantiate();
    auto* const engine { Heliogrim::getEngine() };

    #if USE_MAIN_THREAD_INJECTION
    onEngineCreate(engine);
    #endif

    #if not PREVENT_MAIN_AUTO_SLEEP
    /**
     * Register Shutdown Sequence
     */
    std::atomic_flag sleepFlag {};
    sleepFlag.test_and_set(std::memory_order::release);

    engine->getEmitter().on<SignalShutdownEvent>(
        [sleepFlag = ptr<std::atomic_flag> { &sleepFlag }](cref<SignalShutdownEvent>) {
            sleepFlag->clear(std::memory_order::relaxed);
            sleepFlag->notify_one();
        }
    );
    #endif

    /**
     * Boot Engine
     */
    engine->preInit();
    #if USE_MAIN_THREAD_INJECTION
    onEnginePreInit(engine);
    #endif

    engine->init();
    #if USE_MAIN_THREAD_INJECTION
    onEngineInit(engine);
    #endif

    engine->postInit();
    #if USE_MAIN_THREAD_INJECTION
    onEnginePostInit(engine);
    #endif

    engine->start();
    #if USE_MAIN_THREAD_INJECTION
    onEngineStart(engine);
    #endif

    /**
     * Running State
     */
    #if USE_MAIN_THREAD_INJECTION
    onEngineRunning(engine);
    #endif

    #if not PREVENT_MAIN_AUTO_SLEEP
    /**
     * Set thread asleep
     */
    sleepFlag.wait(true, std::memory_order::consume);
    #endif

    /**
     * Shutdown Engine
     */
    engine->stop();
    #if USE_MAIN_THREAD_INJECTION
    onEngineStop(engine);
    #endif

    engine->shutdown();
    #if USE_MAIN_THREAD_INJECTION
    onEngineShutdown(engine);
    #endif

    engine->exit();
    #if USE_MAIN_THREAD_INJECTION
    onEngineExit(engine);
    #endif

    /**
     * Destroy Engine
     */
    Heliogrim::destroy();

    #if USE_MAIN_THREAD_INJECTION
    afterRoutine();
    #endif

    #ifdef _PROFILING
    __main__stopwatch.stop();
    profiling::Profiler::destroy();
    #endif
}
