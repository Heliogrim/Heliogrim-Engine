#pragma once

#include <atomic>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Meta/Constexpr.hpp>
#include <Engine.Reflect/CompileTypeId.hpp>
#include <Engine.Reflect/Inherit/InheritBase.hpp>

namespace hg::engine::gfx::material {
    template <typename StoredType_>
    struct MaterialParameterStorageTraits {
        using data_type = _STD decay_t<StoredType_>;

        using ref_type = ref<data_type>;
        using cref_type = cref<data_type>;

        using store_type = _STD conditional_t<_STD is_pointer_v<StoredType_>, StoredType_, data_type>;
    };

    template <typename StoredType_>
    struct MaterialParameterStorageTraits<_STD atomic<StoredType_>> {
        using data_type = _STD decay_t<StoredType_>;

        using ref_type = data_type;
        using cref_type = data_type;

        using store_type = _STD atomic<StoredType_>;
    };

    /**/

    class MaterialParameterStorageBase :
        public InheritBase<MaterialParameterStorageBase> {
    public:
        using this_type = MaterialParameterStorageBase;

    protected:
        MaterialParameterStorageBase() noexcept = default;

    public:
        virtual ~MaterialParameterStorageBase() noexcept = default;
    };

    template <typename StoredType_>
    class MaterialParameterStorage :
        public InheritMeta<MaterialParameterStorage<StoredType_>, MaterialParameterStorageBase> {
    public:
        using this_type = MaterialParameterStorage<StoredType_>;

        template <typename Type_>
        using trait_type_t = MaterialParameterStorageTraits<Type_>;
        using trait_type = MaterialParameterStorageTraits<StoredType_>;

    public:
        inline static constexpr type_id typeId { force_constexpr<ctid<this_type>()> };

    public:
        MaterialParameterStorage() noexcept = default;

        ~MaterialParameterStorage() noexcept override = default;

    private:
        typename trait_type::store_type _store;

    public:
        template <typename Tx_ = StoredType_>
        [[nodiscard]] typename trait_type_t<Tx_>::cref_type get() const noexcept {
            return _store;
        }

        template <typename Tx_ = typename trait_type_t<StoredType_>::data_type> requires
            (not _STD is_pointer_v<typename trait_type_t<Tx_>::store_type>) &&
            _STD is_move_assignable_v<typename trait_type_t<Tx_>::data_type>
        void set(typename trait_type_t<Tx_>::data_type&& value_) {
            _store = _STD forward<typename trait_type_t<Tx_>::data_type>(value_);
        }

        template <typename Tx_ = typename trait_type_t<StoredType_>::data_type> requires
            _STD is_pointer_v<typename trait_type_t<Tx_>::store_type> &&
            _STD is_move_assignable_v<typename trait_type_t<Tx_>::store_type> &&
            (
                _STD is_same_v<
                    typename trait_type_t<StoredType_>::store_type, typename trait_type_t<Tx_>::store_type
                > ||
                _STD is_nothrow_convertible_v<
                    typename trait_type_t<Tx_>::store_type,
                    typename trait_type_t<StoredType_>::store_type
                >
            )
        void set(typename trait_type_t<Tx_>::store_type&& value_) {
            _store = _STD forward<typename trait_type_t<Tx_>::store_type>(value_);
        }
    };
}
