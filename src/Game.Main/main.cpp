#include <Windows.h>

#define DEBUG_CPP
#include <Ember.Main/Ember.Main.hpp>
#include <Engine.Common/stdafx.h>
#include <Engine.Resource/File.hpp>
#include <Engine.Scheduler/Task/Task.hpp>
#include <Engine.Scheduler/Thread/Thread.hpp>

#ifdef _PROFILING
#include <Engine.Common/Profiling/Profiler.hpp>
#include <Engine.Common/Profiling/Stopwatch.hpp>
#endif

#include <Ember/Entity.hpp>
#include <Ember/Inbuilt.hpp>
#include <Ember/Level.hpp>
#include <Ember/TextureAsset.hpp>
#include <Ember/Component/Gfx/StaticMeshComponent.hpp>
#include <Engine.Event/GlobalEventEmitter.hpp>
#include <Engine.Event/TickEvent.hpp>
#include <Engine.Scheduler/Async.hpp>
#include <Engine.Session/Session.hpp>

#include "Assets/GfxMaterials/ForestGround01.hpp"
#include "Assets/Meshes/PlaneD128.hpp"
#include "World/Entities/Components/CameraComponent.hpp"

using namespace ember;

void test();

void buildEntity();

void ember_block_main() {

    SCOPED_STOPWATCH

    /**
     * Just delay control flow to hold window open
     */
    engine::scheduler::thread::self::sleepFor(30000);
}

/**
 * Ember main entry
 *
 * @author Julius
 * @date 20.10.2021
 */
void ember_main_entry() {

    SCOPED_STOPWATCH

    /**
     * Generic log of TickEvent
     */
    engine::Session::get()->emitter().on<TickEvent>([](cref<TickEvent> event_) {
        constexpr auto raw { R"(Cycle tick '$0')" };
        DEBUG_NMSG("TickEvent", string(raw).replace(12, 2, _STD to_string(event_.tick)))
    });

    /**
     * Make a repetitive task to emit TickEvent
     */
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

    /**
     * Schedule the tick event
     */
    engine::scheduler::exec(task);

    /**
     *
     */
    // test();
    buildEntity();
}

void test() {

    /**
     *
     */
    {
        Entity entity {};
        auto val = entity::valid(entity);
    }

    /**
     *
     */
    {
        auto possible = entity::create();
        await(possible);

        Entity entity = possible.get();
        auto val = entity::valid(entity);

        await(entity::destroy(_STD move(entity)));
    }

    /**
     *
     */
    {
        auto possible = level::create();
        await(possible);

        Level level = possible.get();
        auto val = level::valid(level);

        await(level::destroy(_STD move(level)));
    }

    /**
     *
     */
    {
        auto possible = level::create();
        await(possible);

        Level level = possible.get();
        auto val = level::valid(level);

        vector<future<Entity>> flist {};
        for (u8 c = 0; c < 128ui8; ++c) {
            flist.push_back(entity::create());
        }

        for (auto& entry : flist) {
            while (!entry.ready()) {
                yield();
            }
        }

        for (auto& entry : flist) {
            level.addEntity(entry.get());
        }

        await(level::destroy(_STD move(level)));
    }
}

void buildEntity() {

    auto possible = entity::create();
    // await(possible);

    Entity entity = possible.get();
    auto val = entity::valid(entity);

    auto& comp = entity.record<component::StaticMeshComponent>();

    comp.setMesh(ember::game::assets::meshes::PlaneD128::auto_guid());
    comp.setMaterial(ember::game::assets::material::ForestGround01::auto_guid());
}
