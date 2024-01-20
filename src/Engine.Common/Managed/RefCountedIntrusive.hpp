#pragma once

#include <memory>
#include <stdexcept>

#include "../Memory/MemoryPointer.hpp"
#include "../Wrapper.hpp"

namespace hg {
    template <typename Ty_>
    concept IsIntrusiveRefCountable = requires(Ty_& obj_) {
            { obj_.ref_inc() };
            { obj_.ref_dec() } -> std::convertible_to<u16>;
        } && requires(const Ty_& obj_) {
            { obj_.ref_count() } -> std::convertible_to<u16>;
        };

    /**/

    template <IsIntrusiveRefCountable Ty_>
    class RefCountedIntrusive;

    /**/

    template <class Derived_>
    class RcFromThis {
    public:
        template <IsIntrusiveRefCountable Ux_>
        friend class RefCountedIntrusive;

    public:
        using this_type = RcFromThis<Derived_>;

        using counter_type = u16;
        using derived_type = Derived_;

    public:
        constexpr RcFromThis() noexcept :
            _refs(1) {
            static_assert(
                std::derived_from<Derived_, this_type>,
                "Used crtp helper class for ref counting incorrectly."
            );
        }

        RcFromThis(const this_type&) = delete;

        RcFromThis(this_type&&) = delete;

        constexpr ~RcFromThis() noexcept = default;

    public:
        ref<this_type> operator=(const this_type&) = delete;

        ref<this_type> operator=(this_type&&) = delete;

    private:
        counter_type _refs;

    public:
        void ref_inc() noexcept {
            ++_refs;
        }

        [[nodiscard]] auto ref_dec() noexcept {
            return --_refs;
        }

        [[nodiscard]] counter_type ref_count() const noexcept {
            return _refs;
        }

    public:
        template <typename Proxy_ = Derived_>
        [[nodiscard]] RefCountedIntrusive<Proxy_> rci_from_this() noexcept;
    };

    /**/

    template <IsIntrusiveRefCountable Ty_>
    class RefCountedIntrusive final {
    public:
        using this_type = RefCountedIntrusive<Ty_>;

        using storage_type = mpt<Ty_>;

    public:
        template <typename... Args_>
        [[nodiscard]] constexpr static this_type create(Args_&&... args_) noexcept {
            return this_type { storage_type::make(std::forward<Args_>(args_)...) };
        }

        [[nodiscard]] constexpr static this_type create_from_storage(ptr<Ty_> self_) noexcept {
            return this_type { storage_type { self_ } };
        }

    private:
        explicit constexpr RefCountedIntrusive(mref<storage_type> storage_) noexcept :
            _obj(std::move(storage_)) {
            // Note: We will obtain the counted reference from the objects constructor
            //inc_not_null();
        }

    public:
        constexpr RefCountedIntrusive() noexcept :
            _obj(nullptr) {}

        constexpr RefCountedIntrusive(std::nullptr_t) noexcept :
            RefCountedIntrusive() {}

        constexpr RefCountedIntrusive(const this_type& other_) noexcept :
            _obj(storage_type { other_._obj.get() }) {
            inc_not_null();
        }

        constexpr RefCountedIntrusive(this_type&& other_) noexcept :
            _obj(std::exchange(other_._obj, storage_type { nullptr })) {}

        constexpr ~RefCountedIntrusive() {
            dec_not_null();
        }

    public:
        ref<this_type> operator=(this_type&& other_) noexcept {
            if (std::addressof(other_) != this) {
                dec_not_null();
                _obj = std::exchange(other_._obj, storage_type { nullptr });
            }
            return *this;
        }

        ref<this_type> operator=(const this_type& other_) noexcept {
            if (std::addressof(other_) != this) {
                dec_not_null();
                _obj = other_._obj;
                inc_not_null();
            }
            return *this;
        }

    private:
        storage_type _obj;

    private:
        constexpr void destroy_obj() {
            _obj.destroy();
            _obj = nullptr;
        }

        constexpr void inc_not_null() noexcept {
            if (_obj != nullptr) {
                _obj->ref_inc();
            }
        }

        constexpr void dec_not_null() {
            if (_obj == nullptr) {
                return;
            }

            if (_obj->ref_dec() == 0) {
                destroy_obj();
            }
        }

        constexpr void throw_if_null() const {
            if (_obj == nullptr) {
                throw std::runtime_error("Failed to access ref counted object with ref count of zero.");
            }
        }

    public:
        [[nodiscard]] auto ref_count() const noexcept {
            using ref_count_type = decltype(std::declval<const Ty_>().ref_count());
            return _obj == nullptr ? ref_count_type { 0 } : _obj->ref_count();
        }

        void reset() {
            dec_not_null();
            _obj = nullptr;
        }

        [[nodiscard]] auto release() noexcept {
            if (ref_count() == 1) {
                return std::exchange(_obj, storage_type { nullptr });
            }
            return storage_type { nullptr };
        }

    public:
        [[nodiscard]] constexpr nmpt<Ty_> get() const noexcept {
            return _obj;
        }

        [[nodiscard]] constexpr ptr<Ty_> operator->() const {
            throw_if_null();
            return _obj.get();
        }

        [[nodiscard]] constexpr cref<Ty_> operator*() const {
            throw_if_null();
            return *_obj;
        }

        [[nodiscard]] constexpr ref<Ty_> operator*() {
            throw_if_null();
            return *_obj;
        }
    };

    /**/

    template <class Derived_>
    template <typename Proxy_>
    RefCountedIntrusive<Proxy_> RcFromThis<Derived_>::rci_from_this() noexcept {
        ref_inc();
        return RefCountedIntrusive<Proxy_>::create_from_storage(static_cast<ptr<Derived_>>(this));
    }
}
