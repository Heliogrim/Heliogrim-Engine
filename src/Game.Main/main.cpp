#include <Engine.Common/__macro.hpp>
#include <Windows.h>

#define DEBUG_CPP
#include <Ember.Main/Ember.Main.hpp>
#include <Engine.Common/stdafx.h>
#include <Engine.Resource/File.hpp>
#include <Engine.Scheduler/Task/Task.hpp>
#include <Engine.Scheduler/Thread/Thread.hpp>
#include <Engine.Logging/Logger.hpp>
#include <Editor.Action/ActionManager.hpp>
#include <Editor.Action/Action/DelegateAction.hpp>

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
#include <Editor.Main/Boot/AssetInit.hpp>

#include "Assets/GfxMaterials/Cannon01.hpp"
#include "Assets/GfxMaterials/Cerberus.hpp"
#include "Assets/GfxMaterials/Dandelion01.hpp"
#include "Assets/GfxMaterials/Skybox01.hpp"
#include "Assets/GfxMaterials/Stick01.hpp"
#include "Assets/GfxMaterials/WoodenPier01Planks.hpp"
#include "Assets/GfxMaterials/WoodenPier01Poles.hpp"
#include "Assets/GfxMaterials/WoodenBucket01.hpp"
#include "Assets/GfxMaterials/WoodenBucket02.hpp"
#include "Assets/Meshes/Cannon01.hpp"
#include "Assets/Meshes/Cerberus.hpp"
#include "Assets/Meshes/Cylinder.hpp"
#include "Assets/Meshes/Dandelion01.hpp"
#include "Assets/Meshes/PlaneD128.hpp"
#include "Assets/Meshes/Rock01.hpp"
#include "Assets/Meshes/Sphere.hpp"
#include "Assets/Meshes/Stick01.hpp"
#include "Assets/Meshes/WoodenPier01Planks.hpp"
#include "Assets/Meshes/WoodenPier01Poles.hpp"
#include "Assets/Meshes/WoodenBucket01.hpp"
#include "Assets/Meshes/WoodenBucket02.hpp"
#include "Ember/Ember.hpp"
#include "Ember/SkyboxComponent.hpp"
#include "Ember/World.hpp"
#include "Engine.Assets/AssetFactory.hpp"
#include "Engine.Assets/Types/GfxMaterial.hpp"
#include "Engine.Assets/Types/Texture/Texture.hpp"
#include "Engine.Assets/Types/Image.hpp"
#include "Engine.Common/Concurrent/Promise.hpp"
#include "Engine.Common/Math/Coordinates.hpp"
#include "Engine.Event/ShutdownEvent.hpp"
#include "Engine.Resource/ResourceManager.hpp"
#include "Engine.GFX.Glow.UI/TestUI.hpp"

#include <Engine.Assets/Types/Texture/Texture.hpp>
#include <Engine.Serialization/Archive/BufferArchive.hpp>
#include <Engine.Serialization/Archive/LayoutArchive.hpp>
#include <Engine.Serialization/Layout/DataLayout.hpp>

using namespace ember;

ptr<Actor> globalPlaneActor = nullptr;

inline static _STD atomic_flag suspended {};

#pragma region Forwarded Functions

void buildTestScene();

void waveActors();

void buildActor(const u64 idx_, const u64 rows_, const u64 cols_);

#pragma endregion

/**
 * Ember main entry
 *
 * @author Julius
 * @date 20.10.2021
 */
void ember_main_entry() {

    SCOPED_STOPWATCH

    /**/
    execute(::ember::editor::boot::initAssets);
    /**/

    /**
     * Create suspend callback for event emitter
     */
    engine::Session::get()->emitter().on<ShutdownEvent>([](cref<ShutdownEvent> event_) {
        suspended.test_and_set(_STD memory_order::release);
    });

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
                IM_DEBUG_LOG("TickEvent", formatted);

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
    #if not defined(_DEBUG) && FALSE
    constexpr u64 rows { 1ui64 << 11 };
    constexpr u64 cols { 1ui64 << 11 };
    constexpr u64 count { rows * cols };

    constexpr u64 perCycle = 4ui64;

    #else
    constexpr u64 rows { 1ui64 << 5 };
    constexpr u64 cols { 1ui64 << 5 };
    constexpr u64 count { rows * cols };

    constexpr u64 perCycle = 4ui64;

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

    /**
     * Generic Animation Task
     */
    RepetitiveTask animTask {
        []() {
            waveActors();
            return !suspended.test(_STD memory_order::consume);
        }
    };

    animTask.srcStage() = TaskStages::eUserUpdateStrong;
    animTask.dstStage() = TaskStages::eUserUpdateStrong;

    /**
     * Generic Generator Task
     */
    RepetitiveTask buildTask {
        [count = count, perCycle = perCycle, animTask = animTask]() {

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

            const auto repeat { idx < count };
            if (!repeat) {
                execute(RepetitiveTask { animTask });
            }
            return repeat;
        }
    };

    buildTask.srcStage() = TaskStages::eUserUpdateStrong;
    buildTask.dstStage() = TaskStages::eUserUpdateStrong;

    execute(_STD move(buildTask));

    //
    execute(buildTestScene);
}

#pragma region Actors Over Time

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

    const auto oxf { _STD sinf(px * math::pi_f) };
    const auto oyf { _STD cosf(py * math::pi_f) };

    position_ += math::vec3_right * (rx * px - rx * 0.5F + oxf * scalar_);
    position_ += math::vec3_forward * (ry * py - ry * 0.5F + oyf * scalar_);
}

#include "Ember/ActorInitializer.hpp"
#include "Ember/StaticGeometryComponent.hpp"
#include "Ember/SkeletalGeometryComponent.hpp"

Vector<Actor*> testActors {};

void buildActor(const u64 idx_, const u64 rows_, const u64 cols_) {

    auto possible = CreateActor(traits::async);
    // await(possible);

    Actor* actor = possible.get();

    // TODO:
    testActors.push_back(actor);

    /**
     *
     */
    auto& initializer { ActorInitializer::get() };
    //initializer.createComponent<ActorComponent>(actor);
    auto* comp = initializer.createComponent<StaticGeometryComponent>(actor);

    /**
     *
     */
    auto queryResult = Ember::assets()[game::assets::meshes::Cylinder::unstable_auto_guid()];
    if (queryResult.flags == AssetDatabaseResultType::eSuccess) {
        comp->setStaticGeometryByAsset(*static_cast<ptr<StaticGeometryAsset>>(&queryResult.value));
    }

    queryResult = Ember::assets()[game::assets::material::Cerberus::unstable_auto_guid()];
    if (queryResult.flags == AssetDatabaseResultType::eSuccess) {
        const auto& list { comp->overrideMaterials() };
        auto& ovm { const_cast<ref<_STD decay_t<decltype(list)>>>(list) };

        ovm.push_back(*static_cast<ptr<GfxMaterialAsset>>(&queryResult.value));
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

    randomPaddedPosition(idx_, rows_, cols_, 3.F, previous);
    const_cast<math::Transform&>(transform).setPosition(previous);
    const_cast<math::Transform&>(transform).setScale(math::vec3 { 0.75F, 0.3F, 0.75F });
    /*
    transform.resolveMatrix();
    entity.setTransform(transform);

    auto comp = entity.record<component::StaticGeometryComponent>();

    comp.setMesh(ember::game::assets::meshes::PlaneD128::auto_guid());
    comp.setMaterial(ember::game::assets::material::ForestGround01::auto_guid());
     */
}

void waveActors() {

    static float timescale { 2000.F };
    static float waveScale { 5.F };
    static float fracScale { 2.F };

    const auto timestamp {
        _STD chrono::duration_cast<_STD chrono::milliseconds>(
            _STD chrono::high_resolution_clock::now().time_since_epoch()).count()
    };

    const float sqrLength { _STD sqrtf(static_cast<float>(testActors.size())) };
    const float fraction { math::pi_f * fracScale / sqrLength };
    const float fracP2 { fraction * fraction };
    const float progress { static_cast<float>(timestamp) / timescale };

    const u32 limit { static_cast<u32>(_STD floor(sqrLength)) };
    for (u32 x { 0ui32 }; x < limit; ++x) {
        for (u32 z { 0ui32 }; z < limit; ++z) {

            const auto idx { x * limit + z };
            Actor* cur { testActors[idx] };

            const auto& transform { cur->getWorldTransform() };
            auto prev { transform.position() };

            const auto xfrac2 { static_cast<float>(x * x) * fracP2 };
            const auto zfrac2 { static_cast<float>(z * z) * fracP2 };
            const auto frac { _STD sqrtf(xfrac2 + zfrac2) };

            const_cast<math::Transform&>(transform).setPosition(prev.setY(_STD sinf(progress + frac) * waveScale));
        }
    }

}

#pragma endregion

#pragma region Simple Assets

ptr<Actor> buildSimpleAsset(asset_guid meshGuid_, asset_guid materialGuid_) {

    auto* actor = await(CreateActor(traits::async));

    auto& initializer { ActorInitializer::get() };
    auto* cmp { initializer.createComponent<StaticGeometryComponent>(actor) };

    auto query { Ember::assets()[meshGuid_] };
    cmp->setStaticGeometryByAsset(*static_cast<ptr<StaticGeometryAsset>>(&query.value));

    query = Ember::assets()[materialGuid_];
    auto& materials { const_cast<ref<_STD decay_t<cref<Vector<GfxMaterialAsset>>>>>(cmp->overrideMaterials()) };
    materials.push_back(*static_cast<ptr<GfxMaterialAsset>>(&query.value));

    return actor;
}

ptr<Actor> buildStick01() {

    auto* actor = buildSimpleAsset(
        game::assets::meshes::Stick01::unstable_auto_guid(),
        game::assets::material::Stick01::unstable_auto_guid()
    );

    cref<math::Transform> transform { actor->getWorldTransform() };
    const_cast<ref<math::Transform>>(transform).setPosition(math::vec3 { 0.F, 1.F, 0.F });
    const_cast<ref<math::Transform>>(transform).setScale(math::vec3 { 10.F });

    GetWorld()->addActor(actor);

    return actor;
}

ptr<Actor> buildWoodenPier01Poles() {

    auto* actor = buildSimpleAsset(
        game::assets::meshes::WoodenPier01Poles::unstable_auto_guid(),
        game::assets::material::WoodenPier01Poles::unstable_auto_guid()
    );

    cref<math::Transform> transform { actor->getWorldTransform() };
    const_cast<ref<math::Transform>>(transform).setPosition(math::vec3 { 0.F, .5F, -1.F });
    const_cast<ref<math::Transform>>(transform).setScale(math::vec3 { 1.F });

    GetWorld()->addActor(actor);

    return actor;
}

ptr<Actor> buildWoodenPier01Planks() {

    auto* actor = buildSimpleAsset(
        game::assets::meshes::WoodenPier01Planks::unstable_auto_guid(),
        game::assets::material::WoodenPier01Planks::unstable_auto_guid()
    );

    cref<math::Transform> transform { actor->getWorldTransform() };
    const_cast<ref<math::Transform>>(transform).setPosition(math::vec3 { -1.F, 0.F, -1.F });
    const_cast<ref<math::Transform>>(transform).setScale(math::vec3 { 1.F });

    GetWorld()->addActor(actor);

    return actor;
}

ptr<Actor> buildWoodenBucket01() {

    auto* actor = buildSimpleAsset(
        game::assets::meshes::WoodenBucket01::unstable_auto_guid(),
        game::assets::material::WoodenBucket01::unstable_auto_guid()
    );

    cref<math::Transform> transform { actor->getWorldTransform() };
    const_cast<ref<math::Transform>>(transform).setPosition(math::vec3 { 0.F, 0.F, .5F });
    const_cast<ref<math::Transform>>(transform).setScale(math::vec3 { 1.F });

    GetWorld()->addActor(actor);

    return actor;
}

ptr<Actor> buildWoodenBucket02() {

    auto* actor = buildSimpleAsset(
        game::assets::meshes::WoodenBucket02::unstable_auto_guid(),
        game::assets::material::WoodenBucket02::unstable_auto_guid()
    );

    cref<math::Transform> transform { actor->getWorldTransform() };
    const_cast<ref<math::Transform>>(transform).setPosition(math::vec3 { 0.F, 0.F, -.5F });
    const_cast<ref<math::Transform>>(transform).setScale(math::vec3 { 1.F });

    GetWorld()->addActor(actor);

    return actor;
}

ptr<Actor> buildDandelion01() {

    auto* actor = buildSimpleAsset(
        game::assets::meshes::Dandelion01::unstable_auto_guid(),
        game::assets::material::Dandelion01::unstable_auto_guid()
    );

    cref<math::Transform> transform { actor->getWorldTransform() };
    const_cast<ref<math::Transform>>(transform).setPosition(math::vec3 { 0.F, .5F, 5.F });
    const_cast<ref<math::Transform>>(transform).setScale(math::vec3 { 5.F });

    GetWorld()->addActor(actor);

    return actor;
}

ptr<Actor> buildCannon01() {

    auto* actor = buildSimpleAsset(
        game::assets::meshes::Cannon01::unstable_auto_guid(),
        game::assets::material::Cannon01::unstable_auto_guid()
    );

    cref<math::Transform> transform { actor->getWorldTransform() };
    const_cast<ref<math::Transform>>(transform).setPosition(math::vec3 { 0.F, 0.F, 0.F });
    const_cast<ref<math::Transform>>(transform).setScale(math::vec3 { 1.F });

    GetWorld()->addActor(actor);

    return actor;
}

#pragma endregion

/**/

#pragma region Skybox Assets

ptr<Actor> buildSkyboxAsset(asset_guid meshGuid_, asset_guid materialGuid_) {

    auto* actor = await(CreateActor(traits::async));

    auto& initializer { ActorInitializer::get() };
    auto* cmp { initializer.createComponent<SkyboxComponent>(actor) };

    auto query { Ember::assets()[meshGuid_] };
    cmp->setSkyboxGeometryByAsset(*static_cast<ptr<StaticGeometryAsset>>(&query.value));

    query = Ember::assets()[materialGuid_];
    cmp->setSkyboxMaterialByAsset(*static_cast<ptr<GfxMaterialAsset>>(&query.value));

    GetWorld()->addActor(actor);

    return actor;
}

#pragma endregion

void buildTestScene() {

    //buildGlobalPlane(12i32);

    //buildRock01();

    auto* skyboxActor = buildSkyboxAsset(
        game::assets::meshes::Sphere::unstable_auto_guid(),
        game::assets::material::Skybox01::unstable_auto_guid()
    );

    auto* actor = buildSimpleAsset(
        game::assets::meshes::Cerberus::unstable_auto_guid(),
        game::assets::material::Cerberus::unstable_auto_guid()
    );

    {
        auto& initializer { ActorInitializer::get() };
        auto* cmp { initializer.createSubComponent<SkeletalGeometryComponent>(actor, actor->getRootComponent()) };

        cref<math::Transform> transform { cmp->getWorldTransform() };
        const_cast<ref<math::Transform>>(transform).setPosition(math::vec3 { 0.F, -1.F, 0.F });
        const_cast<ref<math::Transform>>(transform).setScale(math::vec3 { .2F });
    }

    cref<math::Transform> transform { actor->getWorldTransform() };
    const_cast<ref<math::Transform>>(transform).setPosition(math::vec3 { 0.F, 0.F, 0.F });
    const_cast<ref<math::Transform>>(transform).setScale(math::vec3 { 1.F });

    GetWorld()->addActor(actor);
    ::storeEditorSelectedTarget(actor);

    Vector<ptr<Actor>> sceneActors {};
    sceneActors.push_back(actor);

    /**/

    {
        auto* actor = buildSimpleAsset(
            game::assets::meshes::Cerberus::unstable_auto_guid(),
            game::assets::material::Cerberus::unstable_auto_guid()
        );

        const_cast<ref<math::Transform>>(actor->getWorldTransform()).setPosition(math::vec3 { .8F, -1.F, 0.F });
        const_cast<ref<math::Transform>>(actor->getWorldTransform()).setScale(math::vec3 { .6F });
        const_cast<ref<math::Transform>>(actor->getWorldTransform()).setRotation(math::quaternion::euler({
            0.F,
            glm::radians(-90.F),
            0.F
        }));

        GetWorld()->addActor(actor);
        sceneActors.push_back(actor);

        /**/

        actor = buildSimpleAsset(
            game::assets::meshes::Cerberus::unstable_auto_guid(),
            game::assets::material::Cerberus::unstable_auto_guid()
        );

        const_cast<ref<math::Transform>>(actor->getWorldTransform()).setPosition(math::vec3 { -.8F, -1.F, 0.F });
        const_cast<ref<math::Transform>>(actor->getWorldTransform()).setScale(math::vec3 { .6F });
        const_cast<ref<math::Transform>>(actor->getWorldTransform()).setRotation(math::quaternion::euler({
            0.F,
            glm::radians(90.F),
            0.F
        }));

        GetWorld()->addActor(actor);
        sceneActors.push_back(actor);

        /**/

        actor = buildSimpleAsset(
            game::assets::meshes::Cerberus::unstable_auto_guid(),
            game::assets::material::Cerberus::unstable_auto_guid()
        );

        const_cast<ref<math::Transform>>(actor->getWorldTransform()).setPosition(math::vec3 { 0.F, -1.F, .8F });
        const_cast<ref<math::Transform>>(actor->getWorldTransform()).setScale(math::vec3 { .6F });
        const_cast<ref<math::Transform>>(actor->getWorldTransform()).setRotation(math::quaternion::euler({
            0.F,
            glm::radians(180.F),
            0.F
        }));

        GetWorld()->addActor(actor);
        sceneActors.push_back(actor);

        /**/

        actor = buildSimpleAsset(
            game::assets::meshes::Cerberus::unstable_auto_guid(),
            game::assets::material::Cerberus::unstable_auto_guid()
        );

        const_cast<ref<math::Transform>>(actor->getWorldTransform()).setPosition(math::vec3 { 0.F, -1.F, -.8F });
        const_cast<ref<math::Transform>>(actor->getWorldTransform()).setScale(math::vec3 { .6F });
        const_cast<ref<math::Transform>>(actor->getWorldTransform()).setRotation(math::quaternion::euler({
            0.F,
            glm::radians(0.F),
            0.F
        }));

        GetWorld()->addActor(actor);
        sceneActors.push_back(actor);
    }

    storeHierarchyActor(sceneActors);
}
