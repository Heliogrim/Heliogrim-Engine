#include <Ember/Ember.hpp>

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

    #ifdef WAIT_USER_EXIT
    /**
     * Waiting
     *
     * @author Julius
     * @date 16.01.2020
     *
     * @see Ember::wait()
     */
    Ember::wait();
    #endif

    /**
     * Stop Application
     *
     * @author Julius
     * @date 08.01.2021
     *
     * @see static void Ember::stop()
     */
    Ember::stop();

    #ifndef WAIT_USER_EXIT
    /**
     * Waiting
     *
     * @author Julius
     * @date 16.01.2020
     *
     * @see Ember::wait()
     */
    Ember::wait();
    #endif

    #ifdef _PROFILING
    __main__stopwatch.stop();
    profiling::Profiler::destroy();
    #endif

    /**
     * Return successful execution result
     */
    return 0;
}
