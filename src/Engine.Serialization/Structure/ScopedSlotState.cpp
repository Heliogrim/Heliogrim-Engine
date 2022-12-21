#include "ScopedSlotState.hpp"

#include "StructureSlotState.hpp"

using namespace ember::engine::serialization;
using namespace ember;

bool ScopedSlotState::isImmutable() const noexcept {
    return (flags & ScopedSlotStateFlag::eImmutable) ||
        (rootState->getStateFlags() & StructureSlotStateFlag::eImmutable);
}

bool ScopedSlotState::isScopedImmutable() const noexcept {
    return flags & ScopedSlotStateFlag::eImmutable;
}

bool ScopedSlotState::isDirty() const noexcept {
    return (flags & ScopedSlotStateFlag::eDirty);
    /* (rootState->getStateFlags() & StructureSlotStateFlag::eDirty) */
}

bool ScopedSlotState::isScopedDirty() const noexcept {
    return flags & ScopedSlotStateFlag::eDirty;
}
