#include "Subsystem.hpp"

#include <Engine.ECS/System.hpp>
#include <Engine.Scheduler/Scheduler.hpp>

#include "Components/SkeletalMeshComponent.hpp"
#include "Components/StaticMeshComponent.hpp"
#include "Components/TransformComponent.hpp"

using namespace ember::engine::ecs;
using namespace ember;

inline static _STD atomic_flag _interrupted {};

Subsystem::Subsystem(cref<sptr<Session>> session_, ptr<ecs::System> system_) noexcept :
    _session(session_),
    _system(system_) {}

Subsystem::~Subsystem() noexcept {
    while (_interrupted.test_and_set(_STD memory_order_release)) {
        ;
    }
}

void Subsystem::setupComponents() {
    _system->registerComponent<subsystem::StaticMeshComponent>();
    _system->registerComponent<subsystem::SkeletalMeshComponent>();
    _system->registerComponent<subsystem::TransformComponent>();
}

void Subsystem::setup() {
    setupComponents();

    _system->subsystem() = this;
}

void Subsystem::schedule() {

    auto* shd = static_cast<const ptr<scheduler::Scheduler>>(_session->scheduler());

    shd->exec(scheduler::task::make_repetitive_task(
        [this]() {

            if (_interrupted.test_and_set(_STD memory_order_acq_rel)) {
                _interrupted.clear(_STD memory_order_release);
                return false;
            }

            batchSignaledMaterializes();
            batchSignaledUpdates();

            _interrupted.clear(_STD memory_order_release);
            return true;
        },
        scheduler::task::TaskMask::eNormal,
        scheduler::ScheduleStageBarriers::ePublishStrong,
        scheduler::ScheduleStageBarriers::ePublishStrong
    ));
}

sptr<engine::Session> Subsystem::session() const noexcept {
    return _session;
}

void Subsystem::signalMaterialize(cref<entity_guid> guid_, const component_type_id typeId_,
    const non_owning_rptr<void> ptr_) {
    _signalMaterialize.submit({ guid_, typeId_, ptr_ });
}

void Subsystem::signalUpdate(cref<entity_guid> guid_, const component_type_id typeId_,
    const non_owning_rptr<void> ptr_) {
    _signalUpdate.submit({ guid_, typeId_, ptr_ });
}

void Subsystem::batchSignaledUpdates() {}

void Subsystem::batchSignaledMaterializes() {

    auto& reflections = _system->reflections();

    #if FALSE
    const auto begin = _signalMaterialize.begin();
    const auto end = _signalMaterialize.end();
    const auto diff = end - begin;
    constexpr u64 batch_size = 2048ui64;
    const auto batches = diff / batch_size;

    auto* shd = static_cast<ptr<engine::scheduler::Scheduler>>(_session->scheduler());
    for (u64 i = 1; i < batches; ++i) {

        const auto batchBeg = _STD min(begin + (i - 1) * batch_size, end);
        const auto batchEnd = _STD min(begin + i * batch_size, end);

        _STD vector<subsystem::staged::Submit> _partition { batchBeg, batchEnd };

        shd->exec(engine::scheduler::task::make_task([&reflections, partition = _STD move(_partition)]() {

                for (auto& entry : partition) {
                    auto& re = reflections.at(entry.typeId);
                    /**
                     * Check whether component implements materialize life-cycle callback
                     */
                    if (re.materialize != nullptr) {
                        void (SystemComponentReflectionDummy::*materialize)(cref<entity_guid>) = re.materialize;
                        (static_cast<ptr<SystemComponentReflectionDummy>>(entry.component)->*materialize)(entry.guid);
                    }
                }

            }, scheduler::task::TaskMask::eNormal,
            scheduler::ScheduleStageBarriers::ePublishStrong, scheduler::ScheduleStageBarriers::ePublishStrong));
    }
    #endif

    for (auto& entry : _signalMaterialize) {

        auto& re = reflections.at(entry.typeId);
        /**
         * Check whether component implements materialize life-cycle callback
         */
        if (re.materialize != nullptr) {
            void (SystemComponentReflectionDummy::*materialize)(cref<entity_guid>) = re.materialize;
            (static_cast<ptr<SystemComponentReflectionDummy>>(entry.component)->*materialize)(entry.guid);
        }
    }

    _signalMaterialize.reset();
}
