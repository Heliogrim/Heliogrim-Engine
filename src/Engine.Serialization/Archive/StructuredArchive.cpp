#include "StructuredArchive.hpp"

import Engine.Resource.Archive;

using namespace hg::engine::serialization;
using namespace hg;

StructuredArchive::StructuredArchive(_Inout_ ref<resource::Archive> archive_) :
	_archive(std::addressof(archive_)) {}

ref<engine::resource::Archive> StructuredArchive::getArchive() const noexcept {
	return *_archive.get();
}

RootScopedSlot StructuredArchive::getRootSlot() const {
	return RootScopedSlot { *_archive.get(), ScopedSlotStateFlag::eClean };
}

RootScopedSlot StructuredArchive::insertRootSlot() {
	return RootScopedSlot { *_archive, ScopedSlotStateFlag::eDirty };
}
