#pragma once

#include "LayoutDefine.hpp"
#include "LayoutDefineValueType.hpp"

namespace hg::engine::serialization::layout {
    class LayoutDefineSpan final :
        public LayoutDefine {
    public:
        using this_type = LayoutDefineSpan;
        using type_traits = LayoutDefineValueTrait<LayoutDefineValueType::eSpan>;

    public:
        LayoutDefineSpan(const u64 offset_, cref<sptr<DataLayoutBase>> layout_, const u64 count_);

    protected:
        static ref<serialization::Archive> loadImpl(
            ref<serialization::Archive> archive_,
            cref<LayoutDefine> define_,
            cref<_STD span<u8, _STD dynamic_extent>> dst_
        );

        static ref<serialization::Archive> storeImpl(
            ref<serialization::Archive> archive_,
            cref<LayoutDefine> define_,
            cref<_STD span<u8, _STD dynamic_extent>> src_
        );
    };
}
