#ifdef _PROFILING
#include <Engine.Common/Profiling/Profiler.hpp>
#include <Engine.Common/Profiling/Stopwatch.hpp>
#endif

#include <Ember/Ember.hpp>
#include <Ember/EmberStatic.hpp>

#include <Editor.Core/EditorEngine.hpp>
#include <Engine.Core/Event/SignalShutdownEvent.hpp>
#include <Engine.Event/GlobalEventEmitter.hpp>

using namespace ember;

using SignalShutdownEvent = ::ember::engine::core::SignalShutdownEvent;

int main() {
    #ifdef _PROFILING
    profiling::Profiler::make().startSession("main");
    SCOPED_STOPWATCH_V(__main__stopwatch)
    #endif

    /**
     * Pre-Configure Engine Startup
     */
    EmberStatic::useEngine<editor::EditorEngine>();
    EmberStatic::useSessionGetter<editor::EditorEngine>(&editor::EditorEngine::getPrimaryGameSession);

    /**
     * Instantiate Engine
     */
    Ember::instantiate();
    auto* const engine { Ember::getEngine() };

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

    /**
     * Boot Engine
     */
    engine->preInit();
    engine->init();
    engine->postInit();

    engine->start();

    /**
     * Set thread asleep
     */
    _STD unique_lock<_STD mutex> lck { sleepMtx };
    sleepCnd.wait(lck);

    /**
     * Shutdown Engine
     */
    engine->stop();

    engine->shutdown();
    engine->exit();

    /**
     * Destroy Engine
     */
    Ember::destroy();

    #ifdef _PROFILING
    __main__stopwatch.stop();
    profiling::Profiler::destroy();
    #endif
}
