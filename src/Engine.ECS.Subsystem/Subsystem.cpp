#include "Subsystem.hpp"

#include "Components/StaticMeshComponent.hpp"

using namespace ember::engine::ecs;
using namespace ember;

Subsystem::Subsystem(cref<sptr<Session>> session_, ptr<ecs::registry> registry_) noexcept :
    _session(session_),
    _registry(registry_) {}

Subsystem::~Subsystem() noexcept = default;

void Subsystem::setupComponents() {
    _registry->getOrCreatePool<subsystem::StaticMeshComponent>();
}

void Subsystem::setup() {
    setupComponents();
}

void Subsystem::schedule() {}

sptr<engine::Session> Subsystem::session() const noexcept {
    return _session;
}
