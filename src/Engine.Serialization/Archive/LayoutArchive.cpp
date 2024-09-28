#include "LayoutArchive.hpp"

using namespace hg::engine::serialization;
using namespace hg;

LayoutArchiveBase::LayoutArchiveBase(
	_Inout_ ref<resource::Archive> archive_,
	cref<non_owning_rptr<DataLayoutBase>> layout_
) :
	_archive(std::addressof(archive_)),
	_layout(layout_) {}

LayoutArchiveBase::~LayoutArchiveBase() = default;

nmpt<engine::resource::Archive> LayoutArchiveBase::getArchive() const noexcept {
	return _archive;
}

cref<non_owning_rptr<DataLayoutBase>> LayoutArchiveBase::getDataLayout() const noexcept {
	return _layout;
}
