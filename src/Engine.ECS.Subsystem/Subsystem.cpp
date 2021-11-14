#include "Subsystem.hpp"

#include <Engine.ECS/System.hpp>
#include <Engine.Scheduler/Scheduler.hpp>

#include "Components/SkeletalMeshComponent.hpp"
#include "Components/StaticMeshComponent.hpp"
#include "Components/TransformComponent.hpp"

using namespace ember::engine::ecs;
using namespace ember;

Subsystem::Subsystem(cref<sptr<Session>> session_, ptr<ecs::System> system_) noexcept :
    _session(session_),
    _system(system_) {}

Subsystem::~Subsystem() noexcept = default;

void Subsystem::setupComponents() {
    _system->registerComponent<subsystem::StaticMeshComponent>();
    _stagedProcessor.push(subsystem::StaticMeshComponent::type_id);

    _system->registerComponent<subsystem::SkeletalMeshComponent>();
    _stagedProcessor.push(subsystem::SkeletalMeshComponent::type_id);

    _system->registerComponent<subsystem::TransformComponent>();
}

void Subsystem::setup() {
    setupComponents();
}

void Subsystem::schedule() {

    auto* shd = static_cast<const ptr<scheduler::Scheduler>>(_session->scheduler());

    #if FALSE
    constexpr scheduler::ScheduleStageBarrier barriers[] {
        scheduler::ScheduleStageBarriers::eTopStrong,
        scheduler::ScheduleStageBarriers::eUserUpdateStrong,
        scheduler::ScheduleStageBarriers::eNetworkFetchStrong,
        scheduler::ScheduleStageBarriers::ePhysicsSimulateStrong,
        scheduler::ScheduleStageBarriers::eNetworkPushStrong,
        scheduler::ScheduleStageBarriers::eGraphicNodeCollectStrong,
        scheduler::ScheduleStageBarriers::eBottomStrong
    };

    for (const auto& entry : barriers) {
        shd->exec(scheduler::task::make_repetitive_task(
            [_stagedProcessor = &_stagedProcessor, shd = shd, barrier = entry]() {
                _stagedProcessor->schedule(shd, static_cast<scheduler::ScheduleStage>(barrier.stage));
                return true;
            }, scheduler::task::TaskMask::eNormal, entry, entry));
    }
    #endif
}

sptr<engine::Session> Subsystem::session() const noexcept {
    return _session;
}
