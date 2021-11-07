#pragma once

#include <Engine.Common/Flag.hpp>

namespace ember::engine::scheduler {

    enum class ScheduleStage : ember::u16 {
        eUndefined = 0b0000'0000'0000'0000,
        //
        eTop = 0b0000'0000'0000'0001,
        //
        eUserUpdate = 0b0000'0000'0000'1000,
        //
        eNetworkFetch = 0b0000'0000'0001'0000,
        ePhysicsSimulate = 0b0000'0000'0100'0000,
        eNetworkPush = 0b0000'0001'0000'0000,
        eGraphicNodeCollect = 0b0000'0100'0000'0000,
        //
        eBottom = 0b0100'0000'0000'0000,
        //
        eAll = 0b1000'0000'0000'0000
    };

    using ScheduleStageFlags = Flag<ScheduleStage, ember::u16>;

}
