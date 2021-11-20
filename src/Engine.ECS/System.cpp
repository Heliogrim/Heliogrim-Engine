#include "System.hpp"

#include <cassert>
#include <Engine.Common/Exception/NotImplementedException.hpp>

#include "Engine.ECS.Subsystem/Subsystem.hpp"

using namespace ember::engine::ecs;
using namespace ember;

System::System() noexcept :
    _registry(_STD move(make_uptr<Registry>())) {}

System::~System() noexcept {
    _registry.release();
}

non_owning_rptr<Registry> System::registry() const noexcept {
    return _registry.get();
}

ref<decltype(System::_reflections)> System::reflections() noexcept {
    return _reflections;
}

ptr<void> System::getComponent(cref<component_type_id> typeId_, cref<entity_guid> entity_) const noexcept {

    auto* pool = _registry->get_pool_adapter(typeId_);

    auto* inst = pool->get(entity_);
    return inst;
}

void System::makeComponent(cref<component_type_id> typeId_, cref<entity_guid> entity_) {

    auto* pool = _registry->get_pool_adapter(typeId_);
    pool->insert(entity_);

    auto* inst = pool->get(entity_);
    DEBUG_ASSERT(inst != nullptr, "")

    // TODO:
    _subsystem->signalMaterialize(entity_, typeId_, inst);
}

void System::materializeComponent(cref<component_type_id> typeId_, cref<entity_guid> entity_) {

    auto* pool = _registry->get_pool_adapter(typeId_);
    pool->insert(entity_);

    auto* inst = pool->get(entity_);
    DEBUG_ASSERT(inst != nullptr, "")

    const auto refl = _reflections.at(typeId_);

    /**
     * Check whether component implements materialize life-cycle callback
     */
    if (refl.materialize != nullptr) {
        void (SystemComponentReflectionDummy::*materialize)(cref<entity_guid>) = refl.materialize;
        (static_cast<ptr<SystemComponentReflectionDummy>>(inst)->*materialize)(entity_);
    }
}

void System::materializeComponents(cref<component_type_id> typeId_, cref<vector<entity_guid>> entities_) {

    for (const auto& entry : entities_) {
        materializeComponent(typeId_, entry);
    }
}

void System::destroyComponent(cref<component_type_id> typeId_, cref<entity_guid> entity_) {

    auto* pool = _registry->get_pool_adapter(typeId_);

    auto* inst = pool->get(entity_);
    DEBUG_ASSERT(inst != nullptr, "")

    const auto refl = _reflections.at(typeId_);

    /**
     * Check whether component implements dematerialize life-cycle callback
     */
    if (refl.dematerialize != nullptr) {
        void (SystemComponentReflectionDummy::*dematerialize)(cref<entity_guid>) = refl.dematerialize;
        (static_cast<ptr<SystemComponentReflectionDummy>>(inst)->*dematerialize)(entity_);
    }

    /**
     * Destroy Component
     */
    pool->erase(entity_);
}

void System::destroyComponents(cref<component_type_id> typeId_, cref<vector<entity_guid>> entities_) {

    for (const auto& entry : entities_) {
        destroyComponent(typeId_, entry);
    }
}

void System::destroyAllComponents(cref<component_type_id> typeId_) {
    throw NotImplementedException {};
}

void System::compose(cref<entity_guid> guid_, std::initializer_list<component_type_id> typeIds_) {

    for (const auto& entry : typeIds_) {
        makeComponent(entry, guid_);
    }
}

void System::compose(cref<entity_guid> guid_,
    std::initializer_list<std::pair<component_type_id, ptr<void>>> typeSrcPairs_) {

    // TODO: Replace
    vector<component_type_id> nl {};
    nl.reserve(typeSrcPairs_.size());

    for (const auto& entry : typeSrcPairs_) {
        nl.push_back(entry.first);
    }

    compose(guid_, _STD initializer_list(nl.data(), nl.data() + nl.size()));
}

ref<ptr<Subsystem>> System::subsystem() noexcept {
    return _subsystem;
}
