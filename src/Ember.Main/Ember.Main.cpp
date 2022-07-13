#include "Ember.Main.hpp"

#include <Ember/Ember.hpp>

#ifdef _PROFILING
#include <Engine.Common/Profiling/Profiler.hpp>
#include <Engine.Common/Profiling/Stopwatch.hpp>
#endif

using namespace ember;

/**
 * Main entry-point for this application
 *
 * @author Julius
 * @date 13.10.2021
 *
 * @returns Exit-code for the process - 0 for success, else an error code.
 */
int main() {

    #ifdef _PROFILING
    profiling::Profiler::make().startSession("main");
    SCOPED_STOPWATCH_V(__main__stopwatch)
    #endif

    /**
     * Start Ember Framework
     *
     * @author Julius
     * @date 13.10.2021
     *
     * @see static void Ember::start()
     */
    Ember::launch();

    /**
     * Callback for user defined framework main entry point
     */
    ember_main_entry();

    /**
     * Waiting for Ember Framework to shutdown via exit call of user action
     *  This should happen in any case, cause we need to free every resource correctly acquired by ember session
     *
     * @author Julius
     * @date 16.01.2020
     *
     * @see Ember::wait()
     */
    Ember::wait();

    #ifdef _PROFILING
    __main__stopwatch.stop();
    profiling::Profiler::destroy();
    #endif

    /**
     * Return successful execution result
     */
    return 0;
}
