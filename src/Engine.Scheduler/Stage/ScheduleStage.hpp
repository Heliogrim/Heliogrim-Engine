#pragma once

#include <Engine.Common/__macro.hpp>
#include <Engine.Common/Types.hpp>
#include <Engine.Common/Wrapper.hpp>

#include "ScheduleStage.hpp"

namespace ember::engine::scheduler {

    enum class ScheduleStage : ember::u16 {
        eUndefined = 0b0000'0000'0000'0000,
        //
        eTop = 0b0000'0000'0000'0001,
        //
        eNetworkFetch = 0b0000'0000'0000'1000,
        eUserUpdate = 0b0000'0000'0001'0000,
        //
        ePublish = 0b0000'0000'0010'0000,
        //
        ePhysicsSimulate = 0b0000'0000'0100'0000,
        eNetworkPush = 0b0000'0001'0000'0000,
        eGraphicNodeCollect = 0b0000'0100'0000'0000,
        //
        eBottom = 0b0100'0000'0000'0000,
        //
        eAll = 0b1111'1111'1111'1111
    };

    [[nodiscard]] FORCE_INLINE bool operator==(cref<ScheduleStage> stage_, const ember::u16 value_) noexcept {
        return static_cast<u16>(stage_) == value_;
    }

    [[nodiscard]] FORCE_INLINE bool operator!=(cref<ScheduleStage> stage_, const ember::u16 value_) noexcept {
        return value_ != static_cast<u16>(stage_);
    }

    [[nodiscard]] FORCE_INLINE bool operator==(const ember::u16 value_, cref<ScheduleStage> stage_) noexcept {
        return static_cast<u16>(stage_) == value_;
    }

    [[nodiscard]] FORCE_INLINE bool operator!=(const ember::u16 value_, cref<ScheduleStage> stage_) noexcept {
        return value_ != static_cast<u16>(stage_);
    }

    struct ScheduleStageBarrier {

        constexpr ScheduleStageBarrier() noexcept:
            data(0) {}

        constexpr ScheduleStageBarrier(const u32 data_) noexcept :
            data(data_) {}

        constexpr ScheduleStageBarrier(const ScheduleStage stage_, const bool guarantee_) noexcept :
            stage((ember::u16)stage_),
            __padding__(0),
            guarantee(guarantee_) {}

        union {
            struct {
                u32 data;
            };

            struct {
                u16 stage;
                u8 __padding__;
                bool guarantee;
            };
        };

    public:
        [[nodiscard]] bool weak() const noexcept {
            return !guarantee;
        }

        [[nodiscard]] bool strong() const noexcept {
            return guarantee;
        }
    };

    namespace ScheduleStageOrdering {
        constexpr bool eStrong = true;
        constexpr bool eWeak = false;
    };

    /**
     * Definitions of schedule stage barriers
     *
     * Each stage has a strong and weak definition.
     * Except `ScheduleStage::eUndefined` and `ScheduleStage::eAll`, which are only allowed in certain orders and ignoring ordering
     *
     *  Stage(2) := ScheduleStage \ { eAll, eUndefined }
     *
     * | Src Ordering   | Src Stage         | Dst Ordering  | Dst Stage         | Ctx Stage         | Cnd       | First Cycle   | Last Cycle    | Execution Completed   |
     * | ----           | ----              | ----          | ----              | ----              | ----      | ----          | ----          | ----                  |
     * | `-`            | `eAll`            | `-`           | `eUndefined`      | `-`               | `-`       | `Immediately` | `Undefined`   | `Undefined`           |
     * | `-`            | `eAll`            | `eWeak`       | `Stage (2) :: T`  | `Stage (2) :: C`  | `C != T`  | `Immediately` | End of `T`    | `Undefined`           |
     * | `-`            | `eAll`            | `eStrong`     | `Stage(2) :: T`   | `Stage (2) :: C`  | `C != T`  | `Immediately` | `T`           | End of `T`            |
     * | `eWeak`        | `Stage (2) :: F`  | `-`           | `eUndefined`      | `-`               | `-`       | End of `++F`  | `Undefined`   | `Undefined`           |
     * | `eWeak`        | `Stage (2) :: F`  | `eWeak`       | `Stage (2) :: T`  | `Stage (2) :: C`  | `C != T`  | End of `++F`  | End of `T`    | `Undefined`           |
     * | `eWeak`        | `Stage (2) :: F`  | `eStrong`     | `Stage (2) :: T`  | `-`               | `F != T`  | End of `++F`  | `T`           | End of `T`            |
     * | `eStrong`      | `Stage (2) :: F`  | `-`           | `eUndefined`      | `-`               | `-`       | `F`           | `Undefined`   | `Undefined`           |
     * | `eStrong`      | `Stage (2) :: F`  | `eWeak`       | `Stage (2) :: T`  | `-`               | `-`       | `F`           | End of `T`    | `Undefined`           |
     * | `eStrong`      | `Stage (2) :: F`  | `eStrong`     | `Stage (2) :: T`  | `-`               | `-`       | `F`           | `T`           | End of `T`            |
     *
     * Attention: Currently scheduling with strong guarantee like current stage will only apply to non-repetitive tasks
     * Attention: Repetitive tasks will be scheduled for next UPCOMMING stage
     *
     * Warning: `eAll` is not allowed as dst stage
     * Warning: `eUndefined` is not allowed as src stage
     *
     * Warning: `eAll` has always weak ordering
     * Warning: `eUndefined` has always weak ordering
     *
     * Warning: If src and dst stage are the same, src ordering must be at least as strong as dst ordering
     * Warning: Src ordering must not be weak if src, dst and context stage are the same.
     *
     * Warning: If src ordering is weak and src stage is same as execution context stage, task will be enqueued for next cycle
     * Warning: If src ordering is strong and src stage is same as execution context stage, task will be enqueued for current cycle
     *
     * TODO: Check whether `eUndefined` as src stage should be used to execute immediately and `eAll` to enqueue for next barrier
     */
    namespace ScheduleStageBarriers {
        /**
         *
         */
        constexpr ScheduleStageBarrier eUndefined { 0 };
        /**
         *
         */
        constexpr ScheduleStageBarrier eTopStrong { ScheduleStage::eTop, true };
        constexpr ScheduleStageBarrier eTopWeak { ScheduleStage::eTop, false };
        constexpr ScheduleStageBarrier eNetworkFetchStrong { ScheduleStage::eNetworkFetch, true };
        constexpr ScheduleStageBarrier eNetworkFetchWeak { ScheduleStage::eNetworkFetch, false };
        constexpr ScheduleStageBarrier eUserUpdateStrong { ScheduleStage::eUserUpdate, true };
        constexpr ScheduleStageBarrier eUserUpdateWeak { ScheduleStage::eUserUpdate, false };
        constexpr ScheduleStageBarrier ePublishStrong { ScheduleStage::ePublish, true };
        constexpr ScheduleStageBarrier ePublishWeak { ScheduleStage::ePublish, false };
        constexpr ScheduleStageBarrier ePhysicsSimulateStrong { ScheduleStage::ePhysicsSimulate, true };
        constexpr ScheduleStageBarrier ePhysicsSimulateWeak { ScheduleStage::ePhysicsSimulate, false };
        constexpr ScheduleStageBarrier eNetworkPushStrong { ScheduleStage::eNetworkPush, true };
        constexpr ScheduleStageBarrier eNetworkPushWeak { ScheduleStage::eNetworkPush, false };
        constexpr ScheduleStageBarrier eGraphicNodeCollectStrong { ScheduleStage::eGraphicNodeCollect, true };
        constexpr ScheduleStageBarrier eGraphicNodeCollectWeak { ScheduleStage::eGraphicNodeCollect, false };
        constexpr ScheduleStageBarrier eBottomStrong { ScheduleStage::eBottom, true };
        constexpr ScheduleStageBarrier eBottomWeak { ScheduleStage::eBottom, false };
        /**
         *
         */
        constexpr ScheduleStageBarrier eAll { ScheduleStage::eAll, false };
    };
}
