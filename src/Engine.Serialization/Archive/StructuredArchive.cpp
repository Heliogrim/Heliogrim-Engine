#include "StructuredArchive.hpp"

#include "Archive.hpp"

using namespace ember::engine::serialization;
using namespace ember;

StructuredArchive::StructuredArchive(const ptr<Archive> archive_) :
    _archive(archive_) {}

const ptr<Archive> StructuredArchive::getArchive() const noexcept {
    return _archive;
}

const RootStructureSlot StructuredArchive::getRootSlot() const {
    return RootStructureSlot { _archive };
}

RootStructureSlot StructuredArchive::getRootSlot() {
    return RootStructureSlot { _archive };
}
