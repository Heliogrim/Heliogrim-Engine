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
    _STD atomic_flag sleepFlag {};
    sleepFlag.test_and_set(_STD memory_order::release);

    engine->getEmitter().on<SignalShutdownEvent>(
        [sleepFlag = ptr<_STD atomic_flag> { &sleepFlag }](cref<SignalShutdownEvent>) {
            sleepFlag->clear(_STD memory_order::relaxed);
            sleepFlag->notify_one();
        });

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
    sleepFlag.wait(true, std::memory_order::consume);

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
