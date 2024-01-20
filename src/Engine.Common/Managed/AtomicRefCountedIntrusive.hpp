#pragma once

#include <memory>
#include <stdexcept>

#include "../Wrapper.hpp"

namespace hg {
    template <typename Ty_>
    concept IsAtomicIntrusiveRefCountable = requires(Ty_& obj_) {
            { obj_.ref_inc() } -> std::convertible_to<u16>;
            { obj_.ref_dec() } -> std::convertible_to<u16>;
            { obj_.ref_dec(std::declval<u16>()) } -> std::convertible_to<u16>;
        } && requires(const Ty_& obj_) {
            { obj_.ref_count() } -> std::convertible_to<u16>;
        };

    /**/

    template <IsAtomicIntrusiveRefCountable Ty_>
    class AtomicRefCountedIntrusive;

    /**/

    template <class Derived_>
    class ArcFromThis {
    public:
        template <IsAtomicIntrusiveRefCountable Ux_>
        friend class AtomicRefCountedIntrusive;

    public:
        using this_type = ArcFromThis<Derived_>;

        using counter_type = std::atomic_uint_fast16_t;
        using derived_type = Derived_;

    public:
        constexpr ArcFromThis() noexcept :
            _refs(1) {
            static_assert(
                std::derived_from<Derived_, this_type>,
                "Used crtp helper class for ref counting incorrectly."
            );
        }

        ArcFromThis(const this_type&) = delete;

        ArcFromThis(this_type&&) = delete;

        constexpr ~ArcFromThis() noexcept = default;

    public:
        ref<this_type> operator=(const this_type&) = delete;

        ref<this_type> operator=(this_type&&) = delete;

    private:
        counter_type _refs;

    public:
        [[nodiscard]] auto ref_inc() noexcept {
            return ++_refs;
        }

        [[nodiscard]] auto ref_dec() noexcept {
            return --_refs;
        }

        [[nodiscard]] auto ref_dec(const u16 refs_) noexcept {
            return _refs -= refs_;
        }

        [[nodiscard]] auto ref_count() const noexcept {
            return _refs.load(std::memory_order::consume);
        }

    public:
        template <typename Proxy_ = Derived_>
        [[nodiscard]] AtomicRefCountedIntrusive<Proxy_> arci_from_this() noexcept;
    };

    /**/

    namespace {
        using arci_packed_type = _STD uintptr_t;
        using arci_atomic_packed_type = _STD atomic_uintptr_t;

        constexpr packed_type arci_packed_shift = 16ui64;
        constexpr packed_type arci_packed_ref_mask =
            0b00000000'00000000'00000000'00000000'00000000'00000000'11111111'11111111;
        constexpr packed_type arci_packed_ptr_mask =
            0b11111111'11111111'11111111'11111111'11111111'11111111'00000000'00000000;
    };

    template <IsAtomicIntrusiveRefCountable Ty_>
    class AtomicRefCountedIntrusive final {
    public:
        using this_type = AtomicRefCountedIntrusive<Ty_>;

        using storage_type = arci_atomic_packed_type;

    private:
        [[nodiscard]] static packed_type encode_pointer(const ptr<Ty_> pt_) noexcept {
            return reinterpret_cast<packed_type>(pt_) << packed_shift;
        }

        [[nodiscard]] static packed_type encode_counter(const u16 refs_) noexcept {
            return packed_type { refs_ };
        }

        [[nodiscard]] static Ty_* decode_pointer(const packed_type& packed_) noexcept {
            return reinterpret_cast<Ty_*>((packed_ & arci_packed_ptr_mask) >> arci_packed_shift);
        }

        [[nodiscard]] static u16 decode_counter(const packed_type& packed_) noexcept {
            return static_cast<u16>(packed_ & arci_packed_ref_mask);
        }

        [[nodiscard]] static packed_type encode(const ptr<Ty_> pt_, const u16 refs_) noexcept {
            return encode_pointer(pt_) | encode_counter(refs_);
        }

    public:
        template <typename... Args_>
        [[nodiscard]] constexpr static this_type create(Args_&&... args_) noexcept {

            auto alloc = std::allocator<Ty_> {};
            auto alloc_traits = std::allocator_traits<std::allocator<Ty_>> {};

            auto obj = alloc.allocate(1);
            alloc_traits.construct(alloc, obj, std::forward<Args_>(args_)...);

            return this_type { encode(obj, 1u) };
        }

        [[nodiscard]] constexpr static this_type create_from_storage(const ptr<Ty_> self_) noexcept {
            return this_type { encode(self_, 1u) };
        }

    private:
        explicit constexpr AtomicRefCountedIntrusive(mref<packed_type> storage_) noexcept :
            _obj(storage_) {}

    public:
        constexpr AtomicRefCountedIntrusive() noexcept :
            _obj(0LL) {}

        constexpr AtomicRefCountedIntrusive(std::nullptr_t) noexcept :
            AtomicRefCountedIntrusive() {}

        // TODO:
        constexpr AtomicRefCountedIntrusive(const this_type& other_) noexcept :
            _obj(++const_cast<volatile ref<arci_atomic_packed_type>>(other_._obj)) {

            auto* const subject = decode_pointer(_obj.load(std::memory_order::relaxed));

            if (subject != nullptr) {
                subject->ref_inc();
            }
            --const_cast<volatile ref<arci_atomic_packed_type>>(other_._obj);

            /**/

            _obj.store(encode(subject, 1u), std::memory_order::relaxed);
        }

        // TODO:
        constexpr AtomicRefCountedIntrusive(this_type&& other_) noexcept :
            _obj(other_._obj.exchange(packed_type { 0 }, std::memory_order::release)) {

            auto* const subject = decode_pointer(_obj.load(std::memory_order::relaxed));
            const auto old_refs = decode_counter(_obj.load(std::memory_order::relaxed));

            if (subject != nullptr) {
                subject->ref_inc();
                subject->ref_dec(old_refs);
            }

            _obj.store(encode(subject, 1u), std::memory_order::relaxed);
        }

        constexpr ~AtomicRefCountedIntrusive() {
            dec_not_null();
        }

    public:
        ref<this_type> operator=(this_type&& other_) noexcept = delete;

        ref<this_type> operator=(const this_type& other_) noexcept = delete;

    private:
        storage_type _obj;

    private:
        constexpr void destroy_obj(ptr<Ty_> obj_) {

            auto alloc = std::allocator<Ty_> {};
            auto alloc_traits = std::allocator_traits<std::allocator<Ty_>> {};

            alloc_traits.destroy(alloc, obj_);
            _obj.store(packed_type { 0 }, std::memory_order::relaxed);
        }

        constexpr void cascade_dec(const ptr<Ty_> obj_) {
            if (obj_->ref_dec() == 0) {
                destroy_obj(obj_);
            }
        }

        constexpr void dec_not_null() {

            if (decode_counter(_obj.load(std::memory_order::consume)) == 0) {
                return;
            }

            // Alias: ~fetch_sub (..., std::memory_order::acq_rel)
            const auto packed = --_obj;
            if (decode_counter(packed) == 0) {
                cascade_dec(decode_pointer(packed));
            }
        }

        constexpr void throw_if_null(const packed_type packed_) const {
            if (decode_pointer(packed_) == nullptr) {
                throw std::runtime_error("Failed to access ref counted object with ref count of zero.");
            }
        }

    public:
        [[nodiscard]] auto ref_count() const noexcept {
            using ref_count_type = decltype(std::declval<const Ty_>().ref_count());
            auto* const subject = decode_pointer(_obj.load(std::memory_order::consume));
            return subject == nullptr ? ref_count_type { 0 } : subject->ref_count();
        }

        // TODO:
        void reset() = delete;

        // TODO:
        [[nodiscard]] auto release() noexcept = delete;

    public:
        [[nodiscard]] constexpr ptr<Ty_> operator->() const {
            const auto packed = _obj.load(std::memory_order::consume);
            throw_if_null(packed);
            return decode_pointer(packed);
        }

        [[nodiscard]] constexpr cref<Ty_> operator*() const {
            const auto packed = _obj.load(std::memory_order::consume);
            throw_if_null(packed);
            return *decode_pointer(packed);
        }

        [[nodiscard]] constexpr ref<Ty_> operator*() {
            const auto packed = _obj.load(std::memory_order::consume);
            throw_if_null(packed);
            return *decode_pointer(packed);
        }
    };

    /**/

    template <class Derived_>
    template <typename Proxy_>
    AtomicRefCountedIntrusive<Proxy_> ArcFromThis<Derived_>::arci_from_this() noexcept {
        const auto result = ref_inc();
        assert(result);
        return AtomicRefCountedIntrusive<Proxy_>::create_from_storage(static_cast<ptr<Derived_>>(this));
    }
}
