#include "StructuredArchive.hpp"

#include "Archive.hpp"

using namespace ember::engine::serialization;
using namespace ember;

StructuredArchive::StructuredArchive(const ptr<Archive> archive_) :
    _archive(archive_) {}

const ptr<Archive> StructuredArchive::getArchive() const noexcept {
    return _archive;
}

const RootScopedSlot StructuredArchive::getRootSlot() const {
    return RootScopedSlot { _archive, ScopedSlotStateFlag::eClean };
}

RootScopedSlot StructuredArchive::insertRootSlot() {
    return RootScopedSlot { _archive, ScopedSlotStateFlag::eDirty };
}
