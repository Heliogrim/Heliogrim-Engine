#include "LayoutDefineObject.hpp"

#include <cassert>

#include "DataLayout.hpp"

using namespace hg::engine::serialization::layout;
using namespace hg::engine::serialization;
using namespace hg;

LayoutDefineObject::LayoutDefineObject(const u64 offset_, cref<sptr<DataLayoutBase>> layout_) :
    LayoutDefine(offset_, layout_->size()) {

    _load = this_type::loadImpl;
    _store = this_type::storeImpl;

    _sub = layout_;
}

ref<engine::serialization::Archive> LayoutDefineObject::loadImpl(
    ref<serialization::Archive> archive_,
    cref<LayoutDefine> define_,
    cref<std::span<u8, std::dynamic_extent>> dst_
) {

    const auto& layout { define_._sub };
    // layout->reflect().constructInPlace(&dst_[off]);
    layout->dispatch().load(archive_, dst_.subspan(define_._offset, std::dynamic_extent));

    return archive_;
}

ref<engine::serialization::Archive> LayoutDefineObject::storeImpl(
    ref<serialization::Archive> archive_,
    cref<LayoutDefine> define_,
    cref<std::span<u8, std::dynamic_extent>> src_
) {

    const auto& layout { define_._sub };
    layout->dispatch().store(archive_, src_.subspan(define_._offset, std::dynamic_extent));

    return archive_;
}
