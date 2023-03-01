#include "LayoutArchive.hpp"

using namespace hg::engine::serialization;
using namespace hg;

LayoutArchiveBase::LayoutArchiveBase(const ptr<Archive> archive_, cref<non_owning_rptr<DataLayoutBase>> layout_) :
    _archive(archive_),
    _layout(layout_) {}

LayoutArchiveBase::~LayoutArchiveBase() = default;

const ptr<Archive> LayoutArchiveBase::getArchive() const noexcept {
    return _archive;
}

cref<non_owning_rptr<DataLayoutBase>> LayoutArchiveBase::getDataLayout() const noexcept {
    return _layout;
}
