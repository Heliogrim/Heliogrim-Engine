#pragma once

#include <Engine.Common/Wrapper.hpp>

namespace hg::engine::gfx::material {
    template <typename StoredType_>
    struct MaterialParameterStorageTraits {
        using data_type = _STD decay_t<StoredType_>;

        using ref_type = ref<data_type>;
        using cref_type = cref<data_type>;

        using store_type = data_type;
    };

    template <typename StoredType_>
    class MaterialParameterStorage {
    public:
        using this_type = MaterialParameterStorage<StoredType_>;

        template <typename Type_>
        using trait_type_t = MaterialParameterStorageTraits<Type_>;
        using trait_type = MaterialParameterStorageTraits<StoredType_>;

    private:
        typename trait_type::store_type _store;

    public:
        template <typename Tx_ = StoredType_> requires _STD is_pointer_v<typename trait_type_t<Tx_>::store_type>
        [[nodiscard]] typename trait_type_t<Tx_>::cref_type get() const noexcept {
            return *_store;
        }

        template <typename Tx_ = StoredType_> requires not _STD is_pointer_v<typename trait_type_t<Tx_>::store_type>
        [[nodiscard]] typename trait_type_t<Tx_>::cref_type get() const noexcept {
            return _store;
        }

        template <typename Tx_ = typename trait_type_t<StoredType_>::data_type> requires
            not _STD is_pointer_v<typename trait_type_t<Tx_>::store_type> &&
            _STD is_trivially_move_assignable_v<typename trait_type_t<Tx_>::data_type>
        void set(typename trait_type_t<Tx_>::data_type&& value_) {
            _store = _STD forward<typename trait_type_t<Tx_>::data_type>(value_);
        }

        template <typename Tx_ = typename trait_type_t<StoredType_>::data_type> requires
            _STD is_pointer_v<typename trait_type_t<Tx_>::store_type> &&
            _STD is_move_assignable_v<typename trait_type_t<Tx_>::store_type> &&
            _STD is_pointer_interconvertible_base_of_v<
                typename trait_type_t<StoredType_>::store_type,
                typename trait_type_t<Tx_>::store_type
            >
        void set(typename trait_type_t<Tx_>::store_type&& value_) {
            _store = _STD forward<typename trait_type_t<Tx_>::store_type>(value_);
        }
    };
}
