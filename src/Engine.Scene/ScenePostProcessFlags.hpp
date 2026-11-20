#pragma once

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::scene {
    enum class ScenePostProcessFlagBits : u8;
    typedef Flag<ScenePostProcessFlagBits> ScenePostProcessFlags;
}
