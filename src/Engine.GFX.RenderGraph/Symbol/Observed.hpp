#pragma once

#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Memory/MemoryPointer.hpp>

namespace hg::engine::render::graph {
    class SymbolizedResource;
}

namespace hg::engine::render::graph {
    template <class Type_ = SymbolizedResource, template <typename...> typename StorageType_ = std::type_identity_t>
        requires (not std::is_void_v<Type_>)
    struct Observed {
    public:
        using this_type = Observed<Type_, StorageType_>;

        using value_type = Type_;
        using const_value_type = std::conditional_t<std::is_const_v<Type_>, Type_, std::add_const_t<Type_>>;
        using ref_type = std::conditional_t<std::is_const_v<Type_>, cref<Type_>, ref<Type_>>;
        using const_ref_type = cref<Type_>;

        using storage_type = StorageType_<value_type>;

    public:
        constexpr Observed() noexcept(std::is_nothrow_default_constructible_v<storage_type>) requires
            std::is_default_constructible_v<storage_type> :
            _obj() {}

        template <typename... Args_> requires std::is_constructible_v<storage_type, Args_...>
        constexpr Observed(Args_&&... args_) noexcept(std::is_nothrow_constructible_v<storage_type, Args_...>) :
            _obj(std::forward<Args_>(args_)...) {}

        Observed(mref<this_type>) = delete;

        Observed(cref<this_type>) = delete;

        constexpr ~Observed() noexcept(std::is_nothrow_destructible_v<storage_type>) = default;

    public:
        ref<this_type> operator=(mref<this_type>) = delete;

        ref<this_type> operator=(cref<this_type>) = delete;

    private:
        storage_type _obj;

    public:
        [[nodiscard]] const ptr<const_value_type> obj() const noexcept {
            return std::addressof(_obj);
        }

        [[nodiscard]] const ptr<value_type> obj() noexcept {
            return std::addressof(_obj);
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
