#pragma once

#include "LayoutDefine.hpp"
#include "LayoutDefineValueType.hpp"

namespace hg::engine::serialization::layout {
    template <LayoutDefineValueType ValueType_>
    class LayoutDefineValue final :
        public LayoutDefine {
    public:
        using this_type = LayoutDefineValue<ValueType_>;
        using type_traits = LayoutDefineValueTrait<ValueType_>;

    public:
        LayoutDefineValue(const u64 offset_) :
            LayoutDefine(offset_, type_traits::size) {

            static_assert(type_traits::is_inline && type_traits::is_constant_size && not type_traits::has_subtype, "");

            _load = this_type::loadImpl;
            _store = this_type::storeImpl;
        }

    protected:
        FORCE_INLINE static ref<serialization::Archive> loadImpl(
            ref<serialization::Archive> archive_,
            cref<LayoutDefine> define_,
            cref<_STD span<u8, _STD dynamic_extent>> dst_
        ) {

            using value_type = typename type_traits::type;
            using value_ptr_type = ptr<value_type>;

            return archive_ >> (*reinterpret_cast<value_ptr_type>(&dst_[define_._offset]));
        }

        FORCE_INLINE static ref<serialization::Archive> storeImpl(
            ref<serialization::Archive> archive_,
            cref<LayoutDefine> define_,
            cref<_STD span<u8, _STD dynamic_extent>> src_
        ) {

            using value_type = typename type_traits::type;
            using value_ptr_type = const ptr<value_type>;

            return archive_ << (*reinterpret_cast<value_ptr_type>(&src_[define_._offset]));
        }
    };
}
