#pragma once

#include "../ScheduleWorkSet.hpp"

#include <Engine.Common/Collection/Queue.hpp>
#include <Engine.Common/Concurrent/SpinLock.hpp>

namespace ember::engine::scheduler {
    class FifoWorkSet :
        public ScheduleWorkSet {
    public:
        using this_type = FifoWorkSet;

    public:
        FifoWorkSet();

        ~FifoWorkSet() override;

    private:
        Queue<non_owning_rptr<const task::TaskDelegate>> _fifo;
        concurrent::UnfairSpinLock _lck;

    public:
        [[nodiscard]] bool push(const non_owning_rptr<const task::TaskDelegate> task_) override;

        [[nodiscard]] bool pop(
            const task::TaskMask mask_,
            ref<non_owning_rptr<const task::TaskDelegate>> task_
        ) override;

        void transfer(const non_owning_rptr<ScheduleWorkSet> dst_) override;
    };
}
