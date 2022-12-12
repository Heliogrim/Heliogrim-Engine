#include "Ember.Main.hpp"

#ifdef _PROFILING
#include <Engine.Common/Profiling/Profiler.hpp>
#include <Engine.Common/Profiling/Stopwatch.hpp>
#endif

#include <Ember/Ember.hpp>
#include <Ember/EmberStatic.hpp>

#include <Engine.Core/Event/SignalShutdownEvent.hpp>
#include <Engine.Core.Game/GameEngine.hpp>
#include <Engine.Event/GlobalEventEmitter.hpp>

using namespace ember;

using SignalShutdownEvent = ::ember::engine::core::SignalShutdownEvent;

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
    EmberStatic::useEngine<engine::GameEngine>();
    EmberStatic::useSessionGetter<engine::GameEngine>(&engine::GameEngine::getGameSession);

    /**
     * Instantiate Engine
     */
    Ember::instantiate();
    auto* const engine { Ember::getEngine() };

    #if USE_MAIN_THREAD_INJECTION
    onEngineCreate(engine);
    #endif

    #if not PREVENT_MAIN_AUTO_SLEEP
    /**
     * Register Shutdown Sequence
     */
    _STD condition_variable sleepCnd {};
    _STD mutex sleepMtx {};

    engine->getEmitter().on<SignalShutdownEvent>(
        [sleepMtx = &sleepMtx, sleepCnd = &sleepCnd](cref<SignalShutdownEvent>) {
            sleepMtx->unlock();
            sleepCnd->notify_all();
        });

    sleepMtx.lock();
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
    _STD unique_lock<_STD mutex> lck { sleepMtx };
    sleepCnd.wait(lck);
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
    Ember::destroy();

    #if USE_MAIN_THREAD_INJECTION
    afterRoutine();
    #endif

    #ifdef _PROFILING
    __main__stopwatch.stop();
    profiling::Profiler::destroy();
    #endif
}
