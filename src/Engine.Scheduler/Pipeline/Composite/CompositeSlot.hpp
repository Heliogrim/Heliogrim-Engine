#pragma once

#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Concurrent/SpinLock.hpp>
#include <Engine.Common/Collection/Vector.hpp>

#include "../Stage/StageDispatcher.hpp"
#include "../../Task/TaskCtrl.hpp"

namespace ember::engine::scheduler {
    class CompositeSlot :
        public task::TaskCtrl {
    public:
        using this_type = CompositeSlot;

    public:
        CompositeSlot(
            const non_owning_rptr<class CompositePipeline> pipeline_,
            const non_owning_rptr<class Schedule> schedule_,
            const non_owning_rptr<struct CompositeStage> stage_
        );

        ~CompositeSlot() override;

    private:
        non_owning_rptr<class Schedule> _schedule;
        non_owning_rptr<struct CompositeStage> _stage;

        non_owning_rptr<class CompositePipeline> _pipeline;

    public:
        [[nodiscard]] const non_owning_rptr<class Schedule> getSchedule() const noexcept;

        [[nodiscard]] const non_owning_rptr<struct CompositeStage> getCompositeStage() const noexcept;

        void setCompositeStage(const non_owning_rptr<struct CompositeStage> stage_);

    private:
        mutable concurrent::SpinMutex _mtx;
        Vector<non_owning_rptr<const task::TaskDelegate>> _staticTasks;
        Vector<non_owning_rptr<const task::TaskDelegate>> _dynamicTasks;

    public:
        void staticEnqueue(mref<non_owning_rptr<const task::TaskDelegate>> task_);

        void dynamicEnqueue(mref<non_owning_rptr<const task::TaskDelegate>> task_);

    public:
        [[nodiscard]] StageDispatcher getStaticDispatcher() const noexcept;

        [[nodiscard]] StageDispatcher getDynamicDispatcher() const noexcept;

    private:
        _STD atomic_uint_fast16_t _signals;

    public:
        void onComplete(const non_owning_rptr<const task::TaskDelegate> task_) override;

    public:
        void submit();
    };
}
