#include <Windows.h>

#define DEBUG_CPP
#include <Ember/Ember.hpp>
#include <Engine.Common/stdafx.h>
#include <Engine.Common/Concurrent/Promise.hpp>
#include <Engine.Scheduler/Thread/Thread.hpp>

#if FALSE
#define DR_WAV_IMPLEMENTATION
#include <Engine.SFX/Library/Source/Data/dr_wav.hpp>
#endif

using namespace ember;

/**
 * Main entry-point for this application
 *
 * @author Julius
 * @date 16.01.2020
 *
 * @returns Exit-code for the process - 0 for success, else an error code.
 */
int main() {

    /**
     * Test Case 01 - Promise
     */
    concurrent::promise<uint32_t> p([]() {
        return static_cast<uint32_t>(1);
    });

    p.then<int>([](uint32_t) {
         return 2;
     })
     .then<int>([](int) {
         return 3;
     })
     .then<int8_t>([](int) {
         return 4;
     })
     .then<int16_t>([](int) {
         return 5;
     })
     .then<int32_t>([](int) {
         return 6;
     })
     .then<uint8_t>([](int) {
         return 7;
     })
     .then<uint16_t>([](int) {
         return 8;
     })
     .then<uint32_t>([](int) {
         return 9;
     })
     .finally([](int) {});

    /**
     * Start Application
     *  Create Scheduler
     *  Setup Scheduler
     *  Schedule Construction Calls
     *
     * @author Julius
     * @date 08.01.2021
     *
     * @see static void Ember::stop()
     */
    Ember::start();

    bool interrupt = false;
    while (!interrupt) {
        if constexpr (true) {
            MSG msg {};
            while (GetMessage(&msg, NULL, 0, 0)) {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }

        SDL_Event poll = SDL_Event();
        while (SDL_PollEvent(&poll)) {
            switch (poll.type) {

                case SDL_QUIT: {
                    interrupt = true;
                    break;
                }

                default: break;
            }
        }

        ember::engine::scheduler::thread::self::yield();
    }

    /**
     * Stop Application
     *  Schedule Destruction Calls
     *  Wait for Module Destruction
     *  Destroy Scheduler
     *
     * @author Julius
     * @date 08.01.2021
     *
     * @see static void Ember::stop()
     */
    Ember::stop();

    /**
     * Waiting
     *
     * @author Julius
     * @date 16.01.2020
     *
     * @see GameCore::wait()
     */
    Ember::wait();

    /** Return successful result, so application closes without error */
    return 0;
}
