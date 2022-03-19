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

#include <Ember/Actor.hpp>
#include <Ember/Inbuilt.hpp>
#include <Ember/Level.hpp>
#include <Ember/TextureAsset.hpp>
#include <Engine.Event/GlobalEventEmitter.hpp>
#include <Engine.Event/TickEvent.hpp>
#include <Engine.Session/Session.hpp>

#include "Assets/GfxMaterials/ForestGround01.hpp"
#include "Assets/Meshes/CubeD1.hpp"
#include "Assets/Meshes/PlaneD128.hpp"
#include "Ember/Ember.hpp"
#include "Ember/World.hpp"
#include "Engine.Common/Math/Coordinates.hpp"

using namespace ember;

void test();

void buildActor(const u64 idx_, const u64 rows_, const u64 cols_);

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
     * Make a repetitive task to emit TickEvent
     */
    RepetitiveTask task {
        []() {

            static u64 markCounter = 0ui64;
            ++markCounter;

            // static constexpr double delayFrac = 1. / 60.;
            static constexpr double delayFrac = 1. / .2;
            static constexpr u64 delay = (u64)(delayFrac * 1000000000.0);

            static u64 tmpTick = 0;
            static _STD chrono::high_resolution_clock::time_point tmpNextTick {
                _STD chrono::high_resolution_clock::now()
            };

            static ref<GlobalEventEmitter> emitter = engine::Session::get()->emitter();

            const auto now { _STD chrono::high_resolution_clock::now() };
            if (now >= tmpNextTick) {

                const TickEvent event { tmpTick };
                emitter.emit(event);

                constexpr auto raw { R"(Counted ticks '{}' with '{}' cycles)" };
                auto formatted = std::string("Counted ticks '") + _STD to_string(event.tick) + "' with '" +
                    _STD to_string(markCounter) + "' cycles";
                DEBUG_NMSG("TickEvent", formatted)

                ++tmpTick;
                tmpNextTick = now + _STD chrono::nanoseconds { delay };

                markCounter = 0;
            }

            return true;
        }
    };

    task.srcStage() = TaskStages::eBottomStrong;
    task.dstStage() = TaskStages::eTopStrong;

    execute(_STD move(task));

    /**
     *
     */
    {
        // Is via effect equivalent to commented instruction, cause constructor of inherited asset type class
        //  will autoregister internal created instance to asset database
        game::assets::meshes::PlaneD128 {};
        game::assets::meshes::CubeD1 {};
        /*
        auto* tmp = new game::assets::meshes::PlaneD128 {};
        Ember::assets().insert(tmp);
        delete tmp;
         */
    }

    /**
     * --- [ 4 ] ---
     * rows : 1ui32 << 4 := 16
     * cols : 1ui32 << 4 := 16
     * count : rows * cols := 256
     * 
     * --- [ 5 ] ---
     * rows : 1ui32 << 5 := 32
     * cols : 1ui32 << 5 := 32
     * count : rows * cols := 1024
     * 
     * --- [ 6 ] --
     * rows : 1ui32 << 6 := 64
     * cols : 1ui32 << 6 := 64
     * count : rows * colls := 4096
     */
    // test();
    #ifndef _DEBUG
    constexpr u64 rows { 1ui64 << 11 };
    constexpr u64 cols { 1ui64 << 11 };
    constexpr u64 count { rows * cols };
    #else
    constexpr u64 rows { 1ui64 << 4 };
    constexpr u64 cols { 1ui64 << 4 };
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

    //const u64 perCycle = static_cast<u64>(_STD log10(count));// `_STD log10` is not constexpr
    //constexpr u64 perCycle = count;
    constexpr u64 perCycle = 1ui64;

    RepetitiveTask buildTask {
        [count = count, perCycle = perCycle]() {

            auto timestamp = _STD chrono::high_resolution_clock::now();

            static u64 static_idx = 0;
            static u64 static_log = 0;

            u64& idx = static_idx;
            u64& log = static_log;

            const u64 nextCycleLimit = _STD min(idx + perCycle, count);
            for (; idx < nextCycleLimit; ++idx) {
                buildActor(idx, rows, cols);
            }

            if (idx >= count) {
                std::cout << "Finished creating " << std::to_string(count) << " entities." << std::endl;
            }

            auto end = _STD chrono::high_resolution_clock::now();
            std::cout << "Creating cycle took " << _STD chrono::duration_cast<
                _STD chrono::microseconds>(end - timestamp) << "" << std::endl;

            return idx < count;
        }
    };

    buildTask.srcStage() = TaskStages::eUserUpdateStrong;
    buildTask.dstStage() = TaskStages::eUserUpdateStrong;

    execute(_STD move(buildTask));
}

void test() {

    /**
     *
     */
    {
        Actor* actor = await(CreateActor());
        await(Destroy(_STD move(actor)));
    }

    /**
     *
     */
    {
        ptr<Level> level = await(CreateLevel());
        await(Destroy(_STD move(level)));
    }

    /**
     *
     */
    {
        ptr<Level> level = await(CreateLevel());

        Vector<Future<ptr<Actor>>> flist {};
        for (u8 c = 0; c < 128ui8; ++c) {
            flist.push_back(CreateActor());
        }

        for (auto& entry : flist) {
            while (!entry.ready()) {
                yield();
            }
        }

        /*
        for (auto& entry : flist) {
            level.addEntity(entry.get());
        }
         */

        await(Destroy(_STD move(level)));
    }
}

void randomPaddedPosition(_In_ const u64 idx_, _In_ const u64 rows_, _In_ const u64 cols_, _In_ const float scalar_,
    _Inout_ ref<ember::math::vec3> position_) {

    const float rdxf { static_cast<float>(rows_) };
    const float rdyf { static_cast<float>(cols_) };

    constexpr float padding { 0.25F };
    const float rx { (rdxf + rdxf * padding) * scalar_ };
    const float ry { (rdyf + rdyf * padding) * scalar_ };

    const u64 rowCount = idx_ / cols_;
    const u64 colCount = idx_ - (cols_ * rowCount);

    const float px { static_cast<float>(rowCount) / rdxf };
    const float py { static_cast<float>(colCount) / rdyf };

    const auto oxf { _STD sinf(px * math::pi) };
    const auto oyf { _STD cosf(py * math::pi) };

    position_ += math::vec3_right * (rx * px - rx * 0.5F + oxf * scalar_);
    position_ += math::vec3_forward * (ry * py - ry * 0.5F + oyf * scalar_);
}

#include "Ember/ActorInitializer.hpp"
#include "Ember/StaticGeometryComponent.hpp"

void buildActor(const u64 idx_, const u64 rows_, const u64 cols_) {

    auto possible = CreateActor();
    // await(possible);

    Actor* actor = possible.get();

    /**
     *
     */
    auto& initializer { ActorInitializer::get() };
    //initializer.createComponent<ActorComponent>(actor);
    auto* comp = initializer.createComponent<StaticGeometryComponent>(actor);

    /**
     *
     */
    auto queryResult = Ember::assets()[game::assets::meshes::CubeD1::auto_guid()];
    if (queryResult.flags == AssetDatabaseResultType::eSuccess) {
        comp->setStaticGeometryByAsset(*static_cast<ptr<StaticGeometryAsset>>(&queryResult.value));
    }

    /**
     *
     */
    const auto* root { actor->getRootComponent() };
    const auto& transform { root->getWorldTransform() };

    /*
    auto previous { transform.position() };

    float sig {(idx_ & 0b0001) ? -1.F : 1.F};
    float x {sig * _STD ceilf(static_cast<float>(idx_) / 2.F) * 3.F};

    previous.setX(x);
    const_cast<math::Transform&>(transform).setPosition(previous);
    */

    const ptr<World> world { GetWorld() };
    world->addActor(actor);

    auto previous { transform.position() };

    randomPaddedPosition(idx_, rows_, cols_, 0.75F, previous);
    const_cast<math::Transform&>(transform).setPosition(previous);
    const_cast<math::Transform&>(transform).setScale(math::vec3_one * 0.2F);
    /*
    transform.resolveMatrix();
    entity.setTransform(transform);

    auto comp = entity.record<component::StaticGeometryComponent>();

    comp.setMesh(ember::game::assets::meshes::PlaneD128::auto_guid());
    comp.setMaterial(ember::game::assets::material::ForestGround01::auto_guid());
     */
}
