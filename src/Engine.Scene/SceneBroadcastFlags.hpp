#pragma once
#include <Engine.Common/Flag.hpp>
#include <Engine.Common/Types.hpp>

namespace hg::engine::scene {
    enum class SceneBroadcastFlagBits : u8;
    typedef Flag<SceneBroadcastFlagBits> SceneBroadcastFlags;
}
