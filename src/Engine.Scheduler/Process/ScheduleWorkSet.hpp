#pragma once

#include "../Task/TaskMask.hpp"
#include "../Task/Task.hpp"

namespace ember::engine::scheduler {
    class __declspec(novtable) ScheduleWorkSet {
    public:
        using this_type = ScheduleWorkSet;

    protected:
        ScheduleWorkSet(const task::TaskMask mask_);

    public:
        virtual ~ScheduleWorkSet() = default;

    protected:
        task::TaskMask _mask;

    public:
        [[nodiscard]] task::TaskMask getTaskMask() const noexcept;

        [[nodiscard]] bool contains(const task::TaskMask mask_) const noexcept;

    public:
        [[nodiscard]] virtual bool push(_In_ const non_owning_rptr<const task::TaskDelegate> task_) = 0;

        [[nodiscard]] virtual bool pop(
            _In_ const task::TaskMask mask_,
            _Inout_ ref<non_owning_rptr<const task::TaskDelegate>> task_
        ) = 0;

        virtual void transfer(_In_ const non_owning_rptr<ScheduleWorkSet> dst_) = 0;
    };
}
