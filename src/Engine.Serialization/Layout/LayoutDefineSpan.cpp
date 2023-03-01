#include "LayoutDefineSpan.hpp"

#include <cassert>

#include "DataLayout.hpp"

using namespace hg::engine::serialization::layout;
using namespace hg::engine::serialization;
using namespace hg;

LayoutDefineSpan::LayoutDefineSpan(const u64 offset_, cref<sptr<DataLayoutBase>> layout_, const u64 count_) :
    LayoutDefine(offset_, layout_->size() * static_cast<s64>(count_)) {

    _load = this_type::loadImpl;
    _store = this_type::storeImpl;

    _sub = layout_;
}

ref<engine::serialization::Archive> LayoutDefineSpan::loadImpl(
    ref<serialization::Archive> archive_,
    cref<LayoutDefine> define_,
    cref<std::span<u8, _STD dynamic_extent>> dst_
) {

    const auto& layout { define_._sub };
    const auto size { layout->size() };
    const auto count { define_.size() / size };

    for (s64 i { 0i64 }; i < count; ++i) {

        const auto off { static_cast<s64>(define_._offset) + i * size };

        layout->reflect().constructInPlace(&dst_[off]);
        layout->dispatch().load(archive_, dst_.subspan(off, _STD dynamic_extent));
    }

    return archive_;
}

ref<engine::serialization::Archive> LayoutDefineSpan::storeImpl(
    ref<serialization::Archive> archive_,
    cref<LayoutDefine> define_,
    cref<std::span<u8, _STD dynamic_extent>> src_
) {

    const auto& layout { define_._sub };
    const auto size { layout->size() };
    const auto count { define_.size() / size };

    for (s64 i { 0i64 }; i < count; ++i) {

        const auto off { static_cast<s64>(define_._offset) + i * size };
        layout->dispatch().store(archive_, src_.subspan(off, _STD dynamic_extent));
    }

    return archive_;
}
