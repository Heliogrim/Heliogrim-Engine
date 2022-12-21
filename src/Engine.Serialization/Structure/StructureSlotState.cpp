#include "StructureSlotState.hpp"

using namespace ember::engine::serialization;
using namespace ember;

StructureSlotState::StructureSlotState(const non_owning_rptr<Archive> archive_) :
    _flags(StructureSlotStateFlag::eUndefined),
    _archive(archive_) {}

StructureSlotState::~StructureSlotState() = default;

cref<StructureSlotStateFlags> StructureSlotState::getStateFlags() const noexcept {
    return _flags;
}

ref<StructureSlotStateFlags> StructureSlotState::stateFlags() noexcept {
    return _flags;
}

const non_owning_rptr<Archive> StructureSlotState::getArchive() const noexcept {
    return _archive;
}
