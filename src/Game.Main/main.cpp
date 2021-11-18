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
#include "Engine.Common/Math/Coordinates.hpp"
#include "World/Entities/Components/CameraComponent.hpp"

#include <Engine.GFX/GraphicPass.cpp>

using namespace ember;

void test();

void buildEntity(const u64 idx_, const u64 rows_, const u64 cols_);

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
        },
        engine::scheduler::task::TaskMask::eNormal,
        engine::scheduler::ScheduleStageBarriers::eBottomStrong,
        engine::scheduler::ScheduleStageBarriers::eTopStrong
    );

    /**
     * Schedule the tick event
     */
    engine::scheduler::exec(task);

    /**
     *
     */
    // test();
    #ifndef _DEBUG
    constexpr u64 rows { 1ui64 << 11 };
    constexpr u64 cols { 1ui64 << 11 };
    constexpr u64 count { rows * cols };
    #else
    constexpr u64 rows { 1ui64 << 8 };
    constexpr u64 cols { 1ui64 << 8 };
    constexpr u64 count { rows * cols };
    #endif

    /**
     * [ 5 - 5 ]    : 77MiB
     * [ 6 - 6 ]    : 80MiB
     * [ 7 - 7 ]    : 92MiB
     * [ 8 - 8 ]    : 153MiB
     * [ 9 - 9 ]    : 331MiB
     * [ 10 - 10 ]  : 1,1GiB
     * [ 11 - 11 ]  : 4,1GiB
     * [ 12 - 12 ]  : 16,4GiB
     */

    constexpr u64 progLogThres { count >= (1 << 7) ? count >> 7 : 1 };

    for (u64 idx = 0, log = 0; idx < count; ++idx) {
        //buildEntity(idx, rows, cols);

        if (log * progLogThres == idx) {
            std::cout << "Created " << std::to_string(idx) << " entities." << std::endl;
            ++log;
        }
    }

    std::cout << "Created " << std::to_string(count) << " entities." << std::endl;
    created.test_and_set(_STD memory_order::release);
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

void randomPaddedPosition(_In_ const u64 idx_, _In_ const u64 rows_, _In_ const u64 cols_,
    _Inout_ ref<ember::math::vec3> position_) {

    const u64 rowCount = idx_ / cols_;
    const u64 colCount = idx_ - (cols_ * rowCount);

    constexpr float padding { 0.25F };
    const float rcf { static_cast<float>(rowCount) };
    const float ccf { static_cast<float>(colCount) };
    const float x = ccf + ccf * padding;
    const float y = rcf + rcf * padding;

    const float rf { static_cast<float>(rows_) };
    const float cf { static_cast<float>(cols_) };
    const float gx = cf * cf * padding * .5F;
    const float gy = rf * rf * padding * .5F;

    position_ += math::vec3_right * (gx - x);
    position_ += math::vec3_forward * (gy - y);
}

void buildEntity(const u64 idx_, const u64 rows_, const u64 cols_) {

    auto possible = entity::create();
    // await(possible);

    Entity entity = possible.get();
    auto val = entity::valid(entity);

    auto transform = entity.transform();

    randomPaddedPosition(idx_, rows_, cols_, transform.position());
    transform.scale() = math::vec3_one;

    transform.resolveMatrix();
    entity.setTransform(transform);

    auto comp = entity.record<component::StaticMeshComponent>();

    comp.setMesh(ember::game::assets::meshes::PlaneD128::auto_guid());
    comp.setMaterial(ember::game::assets::material::ForestGround01::auto_guid());
}
