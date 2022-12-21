#include "ScopedSlotGuard.hpp"

#include "ScopedStructureSlot.hpp"

using namespace ember::engine::serialization;
using namespace ember;

ScopedSlotGuard::ScopedSlotGuard(const non_owning_rptr<const ScopedStructureSlotBase> slot_, const bool mutating_) :
    _mutating(mutating_),
    _slot(slot_) {
    const_cast<ptr<ScopedStructureSlotBase>>(_slot)->ensureEntered(_mutating);
}

ScopedSlotGuard::~ScopedSlotGuard() {
    if (_slot) {
        const_cast<ptr<ScopedStructureSlotBase>>(_slot)->ensureLeft(_mutating);
    }
}
