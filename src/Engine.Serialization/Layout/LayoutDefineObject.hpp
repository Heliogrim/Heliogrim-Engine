#pragma once

#include "LayoutDefine.hpp"
#include "LayoutDefineValueType.hpp"

namespace ember::engine::serialization::layout {

    class LayoutDefineObject final :
        public LayoutDefine {
    public:
        using this_type = LayoutDefineObject;
        using type_traits = LayoutDefineValueTrait<LayoutDefineValueType::eObject>;

    public:
        LayoutDefineObject(const u64 offset_, cref<sptr<DataLayoutBase>> layout_);

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
