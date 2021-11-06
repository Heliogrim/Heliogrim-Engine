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
    Ember::start();

    /**
     * Callback for user defined framework main entry point
     */
    ember_main_entry();

    #ifdef WAIT_USER_EXIT
    /**
     * Waiting for Ember Framework to shutdown via exit call of user action
     *
     * @author Julius
     * @date 16.01.2020
     *
     * @see Ember::wait()
     */
    Ember::wait();

    #else
    /**
     * If we don't await framework exit, we need other blocking call or framework will close following control flow
     */
    ember_block_main();
    #endif

    /**
     * Stop Ember Framework
     *  If `WAIT_USER_EXIT` is defined, this will return 'immediately' and ensure stop function call
     *
     * @author Julius
     * @date 08.01.2021
     *
     * @see static void Ember::stop()
     */
    Ember::stop();

    /**
     * Waiting for Ember Framework to shutdown
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
