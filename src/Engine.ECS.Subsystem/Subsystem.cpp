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

    auto repetitive = scheduler::task::make_repetitive_task(
        [_stagedProcessor = &_stagedProcessor, shd = shd]() {

            // TODO: Iterate through schedule stages

            _stagedProcessor->schedule(shd, scheduler::ScheduleStage::eUndefined);
            return true;
        });

    shd->exec(repetitive);
}

sptr<engine::Session> Subsystem::session() const noexcept {
    return _session;
}
