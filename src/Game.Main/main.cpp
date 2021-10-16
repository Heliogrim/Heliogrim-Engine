#include <Windows.h>

#define DEBUG_CPP
#include <Ember/Ember.hpp>
#include <Engine.Common/stdafx.h>
#include <Engine.Common/Concurrent/Promise.hpp>
#include <Engine.Resource/File.hpp>
#include <Engine.Resource/ResourceManager.hpp>
#include <Engine.Resource/Source/FileSource.hpp>
#include <Engine.Scheduler/Task/Task.hpp>
#include <Engine.Scheduler/Thread/Thread.hpp>

#ifdef _PROFILING
#include <Engine.Common/Profiling/Profiler.hpp>
#include <Engine.Common/Profiling/Stopwatch.hpp>
#endif

#include <Engine.Event/GlobalEventEmitter.hpp>
#include <Engine.SFX/Importer/AudioFileTypes.hpp>
#include <Engine.SFX/Importer/SoundImportType.hpp>

#include "Ember/TextureAsset.hpp"
#include "Engine.Event/TickEvent.hpp"
#include "Engine.Scheduler/Async.hpp"
#include "Engine.Session/Session.hpp"
#include "Assets/Textures/GrassWild01Albedo.hpp"

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

    #ifdef _PROFILING
    profiling::Profiler::make().startSession("main");
    SCOPED_STOPWATCH_V(__main__stopwatch)
    #endif

    /**
     * Test Case - Files
     */
    /*
    const string path {
        "R:\\Development\\C++\\Vulkan API\\Game\\resources\\assets\\audio"
    };
    File root { path };
    const bool exists = root.exists();
    const bool dir = root.isDirectory();
    const auto files = root.files();

    const auto& ff = files.front();
    ptr<engine::res::Source> src = new engine::res::FileSource(ff);

    char buffer[1024];
    u64 size = 0;

    const auto success = src->get(0, 1024, buffer, size);
     */

    /**
     * Test Case 01 - Promise
     */
    /*
    concurrent::promise<uint32_t> p([]() {
        return static_cast<uint32_t>(1);
    });

    p.then<int>([](uint32_t) {
        return 2;
    }).then<int>([](int) {
        return 3;
    }).then<int8_t>([](int) {
        return 4;
    }).then<int16_t>([](int) {
        return 5;
    }).then<int32_t>([](int) {
        return 6;
    }).then<uint8_t>([](int) {
        return 7;
    }).then<uint16_t>([](int) {
        return 8;
    }).then<uint32_t>([](int) {
        return 9;
    }).finally([](int) {});
     */

    /**
     * Start the engine framework
     *
     * @author Julius
     * @date 08.01.2021
     *
     * @see static void Ember::start()
     */
    Ember::start();

    auto task = engine::scheduler::task::make_repetitive_task([]() {

        // static constexpr double delayFrac = 1. / 60.;
        static constexpr double delayFrac = 1. / .2;
        static constexpr u64 delay = delayFrac * 1000000000ui64;

        static u64 tmpTick = 0;
        static _STD chrono::high_resolution_clock::time_point tmpNextTick {
            _STD chrono::high_resolution_clock::now()
        };

        static ref<GlobalEventEmitter> emitter = engine::Session::get()->emitter();

        const auto now { _STD chrono::high_resolution_clock::now() };
        if (now >= tmpNextTick) {

            const TickEvent event { tmpTick };
            emitter.emit(event);

            ++tmpTick;
            tmpNextTick = now + _STD chrono::nanoseconds { delay };
        }

        return true;
    });

    engine::scheduler::exec(task);

    engine::Session::get()->emitter().on<TickEvent>([](cref<TickEvent> event_) {
        DEBUG_MSG("TickEvent")
    });

    /**
     *
     */
    {
        //
        auto wdb = Ember::assets();
        auto result = wdb.operator[]<TextureAsset>(game::assets::texture::GrassWild01Albedo::auto_guid());

        assert(result);
        TextureAsset asset = result.value;
    }

    /**
     * Test Case - Watcher
     */
    #if FALSE
    {
        const string path = R"(R:\\Development\\C++\\Vulkan API\\Game\\resources\\assets\\audio)";
        File file { path };

        auto rm = engine::ResourceManager::get();
        auto idx = rm->indexer();

        idx->on([&](cref<File> file_) {

            const auto type = engine::res::FileTypeRegister::get().getByExt(
                _STD filesystem::path { file_.url() }.extension().string());

            if (!type.valid()) {
                return false;
            }

            if (type == engine::sfx::AudioFileType::Wav) {

                auto fr = rm->importer().import<engine::sfx::SoundImportType>(type, file_);
                auto result { _STD move(fr.get()) };

                delete result.buffer.mem;
            }

            return true;
        });

        idx->scan(file);
    }
    #endif

    engine::scheduler::thread::self::sleepFor(5000);

    bool interrupt = false;
    while (!interrupt) {

        if constexpr (true) {
            _STD this_thread::sleep_for(_STD chrono::milliseconds { 15000 });
            ember::engine::scheduler::thread::self::yield();
            interrupt = true;
        }
    }

    /**
     * Stops the engine framework
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

    #ifdef _PROFILING
    __main__stopwatch.stop();
    profiling::Profiler::destroy();
    #endif

    /** Return successful result, so application closes without error */
    return 0;
}
