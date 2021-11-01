#include "Subsystem.hpp"

#include "Components/StaticMeshComponent.hpp"
#include "Components/TransformComponent.hpp"
#include "Engine.ECS/System.hpp"

using namespace ember::engine::ecs;
using namespace ember;

Subsystem::Subsystem(cref<sptr<Session>> session_, ptr<ecs::System> system_) noexcept :
    _session(session_),
    _system(system_) {}

Subsystem::~Subsystem() noexcept = default;

void Subsystem::setupComponents() {
    _system->registerComponent<subsystem::StaticMeshComponent>();
    _system->registerComponent<subsystem::TransformComponent>();
}

void Subsystem::setup() {
    setupComponents();
}

void Subsystem::schedule() {}

sptr<engine::Session> Subsystem::session() const noexcept {
    return _session;
}
