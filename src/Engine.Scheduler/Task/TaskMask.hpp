#pragma once
#include <Engine.Common/Flag.hpp>
#include <Engine.Common/Types.hpp>

namespace ember::engine::scheduler::task {

    enum class TaskMask: ember::u8 {
        eUndefined = 0b0,
        //
        eCritical = 0b1 << 0,
        eHigher = 0b1 << 1,
        eNormal = 0b1 << 2,
        eLower = 0b1 << 3,

        //
        eIO = 0b1 << 4,

        // Concatenated
        eAll = 0b11111111
    };

    [[nodiscard]] FORCE_INLINE bool operator&(const TaskMask& left_, const TaskMask& right_) noexcept {
        return static_cast<u8>(left_) & static_cast<u8>(right_);
    }

    typedef Flag<TaskMask> TaskMasks;

    constexpr TaskMask TaskMaskValues[] {
        TaskMask::eUndefined,
        TaskMask::eCritical,
        TaskMask::eHigher,
        TaskMask::eNormal,
        TaskMask::eLower,
        TaskMask::eIO
    };
}
