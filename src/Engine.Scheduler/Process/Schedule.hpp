#pragma once

#include <Engine.Common/Collection/Array.hpp>
#include <Engine.Common/Wrapper.hpp>

#include "../Task/TaskMask.hpp"
#include "ScheduleWorkSet.hpp"

namespace ember::engine::scheduler {
    namespace {
        inline constexpr static task::TaskMask priorities[] {
            task::TaskMask::eCritical,
            task::TaskMask::eHigher,
            task::TaskMask::eNormal,
            task::TaskMask::eLower
        };

        inline constexpr static u64 priority_count = sizeof(priorities) / sizeof(task::TaskMask);
    }

    class Schedule {
    public:
        using this_type = Schedule;

    public:
        Schedule();

        ~Schedule();

    public:
        void setup();

        void tick();

        void tidy();

    private:
        Array<_STD atomic<sptr<ScheduleWorkSet>>, priority_count> _workSets;

    private:
        [[nodiscard]] u64 maskToPriority(const task::TaskMask mask_) const noexcept;

        void transfer(const task::TaskMask src_, const task::TaskMask dst_);

    public:
        [[nodiscard]] bool push(_In_ const non_owning_rptr<const task::TaskDelegate> task_);

        [[nodiscard]] bool pop(
            _In_ const task::TaskMask mask_,
            _Inout_ ref<non_owning_rptr<const task::TaskDelegate>> task_
        );
    };
}
