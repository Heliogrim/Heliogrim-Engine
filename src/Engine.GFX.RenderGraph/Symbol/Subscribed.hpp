#pragma once

#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Memory/MemoryPointer.hpp>

namespace hg::engine::gfx::render::graph {
    class SymbolizedResource;
}

namespace hg::engine::gfx::render::graph {
    template <class Type_ = SymbolizedResource> requires (not _STD is_void_v<Type_>)
    struct ALIGNED(Subscribed, sizeof(ptr<Type_>)) {
    public:
        using this_type = Subscribed<Type_>;

        using value_type = Type_;
        using const_value_type = _STD conditional_t<_STD is_const_v<Type_>, Type_, _STD add_const_t<Type_>>;
        using ref_type = _STD conditional_t<_STD is_const_v<Type_>, cref<Type_>, ref<Type_>>;
        using const_ref_type = cref<Type_>;

    public:
        constexpr Subscribed() noexcept :
            _obj(nullptr) {}

        Subscribed(mref<this_type>) = delete;

        Subscribed(cref<this_type>) = delete;

        constexpr ~Subscribed() noexcept = default;

    public:
        ref<this_type> operator=(mref<this_type>) = delete;

        ref<this_type> operator=(cref<this_type>) = delete;

    private:
        volatile ptr<Type_> _obj;

    private:
        [[nodiscard]] const ptr<const_value_type> obj() const noexcept {
            return _obj;
        }

        [[nodiscard]] const ptr<value_type> obj() noexcept {
            return _obj;
        }

    public:
        [[nodiscard]] bool empty() const noexcept {
            return _obj == nullptr;
        }

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

    static_assert(sizeof(Subscribed<size_t>) == alignof(ptr<size_t>));
}
