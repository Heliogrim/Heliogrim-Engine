#pragma once
#include <Engine.Common/String.hpp>
#include <Engine.Common/Types.hpp>

namespace hg::engine::accel {
    enum class DefinitionType : u8 {
        eInvalid = 0x0,
        /**/
        eText = 0x1,
        eInclude = 0x2,
        /**/
        eUnsupported = 0xF
    };

    struct ProfileDefinition {
        DefinitionType type { DefinitionType::eInvalid };
        string data;
    };
}
