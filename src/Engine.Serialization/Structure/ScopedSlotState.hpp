#pragma once

#include <Engine.Common/Types.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Flag.hpp>

namespace ember::engine::serialization {
    class ScopedStructureSlotBase;
    class StructureSlotState;
}

namespace ember::engine::serialization {
    enum class ScopedSlotStateFlag : u8 {
        eUndefined = 0x0,
        /**/
        eImmutable,
        eDirty
    };

    typedef Flag<ScopedSlotStateFlag, u8> ScopedSlotStateFlags;

    struct ScopedSlotState {
        ScopedSlotStateFlags flags;
        cref<ScopedStructureSlotBase> parent;

        sptr<StructureSlotState> rootState;

    public:
        [[nodiscard]] bool isImmutable() const noexcept;

        [[nodiscard]] bool isScopedImmutable() const noexcept;

        [[nodiscard]] bool isDirty() const noexcept;

        [[nodiscard]] bool isScopedDirty() const noexcept;
    };
}
