#pragma once

#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Memory/MemoryPointer.hpp>

namespace hg::engine::render::graph {
    class SymbolizedResource;
}

namespace hg::engine::render::graph {
    template <class Type_ = SymbolizedResource, template <typename...> typename StorageType_ = _STD type_identity_t>
        requires (not _STD is_void_v<Type_>)
    struct Observed {
    public:
        using this_type = Observed<Type_, StorageType_>;

        using value_type = Type_;
        using const_value_type = _STD conditional_t<_STD is_const_v<Type_>, Type_, _STD add_const_t<Type_>>;
        using ref_type = _STD conditional_t<_STD is_const_v<Type_>, cref<Type_>, ref<Type_>>;
        using const_ref_type = cref<Type_>;

        using storage_type = StorageType_<value_type>;

    public:
        template <class Tx_ = storage_type> requires _STD is_default_constructible_v<Tx_>
        constexpr Observed() noexcept(_STD is_nothrow_default_constructible_v<Tx_>) :
            _obj() {}

        template <class Tx_ = storage_type> requires _STD is_move_constructible_v<Tx_>
        constexpr Observed(mref<Tx_> subject_) noexcept(_STD is_nothrow_move_constructible_v<Tx_>) :
            _obj(_STD move(subject_)) {}

        template <class Tx_ = storage_type> requires _STD is_copy_constructible_v<Tx_>
        constexpr Observed(cref<Tx_> subject_) noexcept(_STD is_nothrow_copy_constructible_v<Tx_>) :
            _obj(subject_) {}

        template <class Tx_ = storage_type, typename... Args_> requires _STD is_constructible_v<Tx_, Args_...>
        constexpr Observed(Args_&&... args_) noexcept(_STD is_nothrow_constructible_v<Tx_, Args_...>) :
            _obj(_STD forward<Args_>(args_)...) {}

        Observed(mref<this_type>) = delete;

        Observed(cref<this_type>) = delete;

        constexpr ~Observed() noexcept(_STD is_nothrow_destructible_v<storage_type>) = default;

    public:
        ref<this_type> operator=(mref<this_type>) = delete;

        ref<this_type> operator=(cref<this_type>) = delete;

    private:
        storage_type _obj;

    public:
        [[nodiscard]] const ptr<const_value_type> obj() const noexcept {
            return _STD addressof(_obj);
        }

        [[nodiscard]] const ptr<value_type> obj() noexcept {
            return _STD addressof(_obj);
        }

    public:
        [[nodiscard]] nmpt<const_value_type> operator->() const noexcept {
            return obj();
        }

        [[nodiscard]] nmpt<value_type> operator->() noexcept {
            return obj();
        }

        [[nodiscard]] const_ref_type operator*() const noexcept {
            return *obj();
        }

        [[nodiscard]] ref_type operator*() noexcept {
            return *obj();
        }
    };
}
