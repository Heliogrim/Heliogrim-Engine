#pragma once

#include <atomic>
#include <concepts>
#include <type_traits>
#include <xmemory>

#include "__fwd.hpp"

#include "../Wrapper.hpp"
#include "../Meta/Concept.hpp"

namespace hg {
    template <typename Ty_, typename AllocType_, bool Atomic_>
    struct MemoryPointerStorage;

    template <typename Ty_, typename AllocType_>
    struct MemoryPointerStorage<Ty_, AllocType_, true> {
        using this_type = MemoryPointerStorage<Ty_, AllocType_, true>;

        using allocator_type = AllocType_;
        using allocator_traits = _STD allocator_traits<allocator_type>;

        using mem_type = _STD atomic<Ty_*>;

    public:
        constexpr MemoryPointerStorage() noexcept :
            mem(nullptr) {}

        constexpr MemoryPointerStorage(nullptr_t) noexcept :
            mem(nullptr) {}

        MemoryPointerStorage(cref<this_type> other_) :
            mem(other_.mem.load(_STD memory_order::seq_cst)) {}

        MemoryPointerStorage(mref<this_type> other_) :
            mem(other_.mem.exchange(nullptr, _STD memory_order::seq_cst)) {}

        ~MemoryPointerStorage() = default;

    public:
        ref<this_type> operator=(cref<this_type> other_) noexcept {
            if (_STD addressof(other_) != this) {
                mem.store(other_.mem.load(_STD memory_order::acquire), _STD memory_order::release);
            }
            return *this;
        }

        ref<this_type> operator=(mref<this_type> other_) noexcept {
            if (_STD addressof(other_) != this) {
                mem.store(other_.mem.exchange(nullptr, _STD memory_order::seq_cst), _STD memory_order::release);
            }
            return *this;
        }

    public:
        [[nodiscard]] constexpr static bool isAtomic() noexcept {
            return _STD bool_constant<true>::value;
        }

    public:
        template <typename... Args_>
        void allocateWith(allocator_type&& allocator_, Args_&&... args_) {

            /* Prepare next subject within local scope to prevent unattended updates */
            auto* nextSnapshot = allocator_.allocate(1);
            _STD construct_at<Ty_, Args_...>(nextSnapshot, _STD forward<Args_>(args_)...);

            Ty_* expected = nullptr;
            if (not mem.compare_exchange_strong(expected, nextSnapshot, _STD memory_order_seq_cst)) {
                /* Rollback on operation failure */
                allocator_traits::destroy(allocator_, nextSnapshot);
                allocator_.deallocate(nextSnapshot, 1);
            }
        }

        template <typename... Args_> requires _STD is_constructible_v<Ty_, Args_...>
        void allocate(Args_&&... args_) {
            allocateWith<Args_...>(allocator_type {}, _STD forward<Args_>(args_)...);
        }

        void deallocate(AllocType_&& allocator_ = AllocType_ {}) {

            using allocator_traits = _STD allocator_traits<AllocType_>;

            Ty_* expectation = nullptr;
            auto* snapshot = mem.exchange(expectation, _STD memory_order::seq_cst);

            if (snapshot != nullptr) {
                allocator_traits::destroy(allocator_, snapshot);
                allocator_.deallocate(snapshot, 1);
            }
        }

    public:
        template <typename Tx_ = Ty_> requires _STD is_move_assignable_v<Ty_> && _STD is_convertible_v<Tx_, Ty_>
        void store(Tx_&& value_) noexcept {
            (*mem.load(_STD memory_order::consume)) = _STD forward<Tx_>(value_);
        }

        template <typename Tx_ = Ty_> requires _STD is_move_assignable_v<Ty_> && _STD is_convertible_v<Tx_, Ty_>
        bool safeStore(Tx_&& value_) noexcept {

            auto* const snapshot = mem.load(_STD memory_order::consume);
            if (snapshot == nullptr) {
                return false;
            }

            (*snapshot) = _STD forward<Tx_>(value_);
            return true;
        }

        template <typename Tx_ = Ty_>
        const Tx_* const load() const noexcept {
            return mem.load(_STD memory_order::consume);
        }

        template <typename Tx_ = Ty_>
        Tx_* const load() noexcept {
            return mem.load(_STD memory_order::consume);
        }

    public:
        template <typename Tx_ = Ty_> requires _STD is_same_v<Tx_, Ty_> && _STD is_default_constructible_v<Tx_>
        Tx_* const loadOrAllocate() {

            if (mem.load(_STD memory_order_consume) == nullptr) {
                allocate<>();
            }

            return load();
        }

        template <typename Ptx_ = Ty_*> requires _STD is_convertible_v<Ptx_, Ty_*>
        [[nodiscard]] Ty_* exchange(Ptx_&& next_) {

            Ty_* expected = nullptr;
            while (mem.compare_exchange_strong(expected, next_, _STD memory_order::seq_cst)) {
                // No-op
            }

            return expected;
        }

        template <typename Ptx_ = Ty_*> requires std::is_convertible_v<Ptx_, Ty_*>
        [[nodiscard]] std::pair<bool, Ty_*> compare_exchange(const ptr<Ty_> expect_, Ptx_&& next_) {
            Ty_* expect = expect_;
            const auto success = mem.compare_exchange_strong(
                expect_,
                std::forward<Ptx_>(next_),
                std::memory_order::acq_rel
            );
            return std::make_pair(success, expect);
        }

    public:
        mem_type mem;
    };

    template <typename Ty_, typename AllocType_>
    struct MemoryPointerStorage<Ty_, AllocType_, false> {
        using this_type = MemoryPointerStorage<Ty_, AllocType_, false>;

        using allocator_type = AllocType_;
        using allocator_traits = _STD allocator_traits<allocator_type>;

        using mem_type = Ty_*;

    public:
        constexpr MemoryPointerStorage() noexcept :
            mem(nullptr) {}

        constexpr MemoryPointerStorage(nullptr_t) noexcept :
            mem(nullptr) {}

        constexpr MemoryPointerStorage(mem_type value_) noexcept :
            mem(value_) {}

        template <typename Tx_, typename AllocTx_> requires _STD is_convertible_v<Tx_*, Ty_*>
        constexpr MemoryPointerStorage(cref<MemoryPointerStorage<Tx_, AllocTx_, false>> other_) noexcept :
            mem(other_.mem) {}

        constexpr MemoryPointerStorage(cref<this_type> other_) noexcept :
            mem(other_.mem) {}

        template <typename Tx_, typename AllocTx_> requires _STD is_nothrow_convertible_v<Tx_*, Ty_*>
        constexpr MemoryPointerStorage(mref<MemoryPointerStorage<Tx_, AllocTx_, false>> other_) noexcept :
            mem(_STD exchange(other_.mem, nullptr)) {}

        constexpr MemoryPointerStorage(mref<this_type> other_) noexcept :
            mem(_STD exchange(other_.mem, nullptr)) {}

        ~MemoryPointerStorage() = default;

    public:
        ref<this_type> operator=(cref<this_type> other_) noexcept {
            if (_STD addressof(other_) != this) {
                mem = other_.mem;
            }
            return *this;
        }

        ref<this_type> operator=(mref<this_type> other_) noexcept {
            if (_STD addressof(other_) != this) {
                mem = _STD exchange(other_.mem, nullptr);
            }
            return *this;
        }

    public:
        [[nodiscard]] constexpr static bool isAtomic() noexcept {
            return _STD bool_constant<false>::value;
        }

    public:
        template <typename... Args_> requires _STD is_constructible_v<Ty_, Args_...>
        constexpr void allocateWith(allocator_type&& allocator_, Args_&&... args_) {

            auto* nextSnapshot = allocator_.allocate(1);
            _STD construct_at<Ty_, Args_...>(nextSnapshot, _STD forward<Args_>(args_)...);

            mem = _STD move(nextSnapshot);
        }

        template <typename... Args_> requires _STD is_constructible_v<Ty_, Args_...>
        constexpr void allocate(Args_&&... args_) {
            allocateWith<Args_...>(allocator_type {}, _STD forward<Args_>(args_)...);
        }

        void deallocate(allocator_type&& allocator_ = AllocType_ {}) {

            auto* snapshot = _STD exchange(mem, nullptr);

            if (snapshot != nullptr) {
                allocator_traits::destroy(allocator_, snapshot);
                allocator_.deallocate(snapshot, 1);
            }
        }

    public:
        template <typename Tx_ = Ty_> requires
            (not _STD is_const_v<Ty_>) && _STD is_move_assignable_v<Ty_> && _STD is_convertible_v<Tx_, Ty_>
        void store(Tx_&& value_) noexcept {
            (*mem) = _STD forward<Tx_>(value_);
        }

        template <typename Tx_ = Ty_> requires
            (not _STD is_const_v<Ty_>) && _STD is_move_assignable_v<Ty_> && _STD is_convertible_v<Tx_, Ty_>
        bool safeStore(Tx_&& value_) noexcept {
            if (mem != nullptr) {
                (*mem) = _STD forward<Tx_>(value_);
                return true;
            }
            return false;
        }

        template <typename Tx_ = Ty_>
        constexpr Tx_* const load() const noexcept {
            return mem;
        }

        template <typename Tx_ = Ty_> requires (not _STD is_const_v<Ty_>)
        constexpr Tx_* const load() noexcept {
            return mem;
        }

    public:
        template <typename Tx_ = Ty_> requires _STD is_same_v<Tx_, Ty_> && _STD is_default_constructible_v<Tx_>
        Tx_* const loadOrAllocate() {

            if (mem == nullptr) {
                allocate<>();
            }

            return load();
        }

        template <typename Ptx_ = Ty_*> requires _STD is_convertible_v<_STD remove_cvref_t<Ptx_>, Ty_*>
        [[nodiscard]] constexpr Ty_* exchange(Ptx_&& next_) {
            return _STD exchange(mem, _STD forward<Ptx_>(next_));
        }

    public:
        mem_type mem;
    };

    template <typename Ty_, typename AllocType_, typename StorageType_>
    struct MemoryPointer {
    public:
        using this_type = MemoryPointer<Ty_, AllocType_, StorageType_>;

        using storage_type = StorageType_;

        using allocator_type = AllocType_;
        using allocator_traits = _STD allocator_traits<allocator_type>;

        using address_type = _STD ptrdiff_t;
        using difference_type = _STD ptrdiff_t;

        using non_owning_type = NonOwningMemoryPointer<Ty_, storage_type>;

    public:
        constexpr MemoryPointer() noexcept :
            storage() {}

        template <typename Tx_ = Ty_> requires
            _STD is_nothrow_convertible_v<Tx_*, Ty_*>
        constexpr MemoryPointer(Tx_* value_) noexcept :
            storage(value_) {}

        constexpr MemoryPointer(std::nullptr_t) noexcept :
            storage(nullptr) {}

        /* Weak protection against shared ownership for raw pointers */
        constexpr MemoryPointer(cref<this_type>) = delete;

        constexpr MemoryPointer(mref<this_type> other_) noexcept :
            storage(_STD move(other_.storage)) {}

        ~MemoryPointer() = default;

    public:
        /* Weak protection against shared ownership for raw pointers */
        ref<this_type> operator=(cref<this_type>) = delete;

        ref<this_type> operator=(mref<this_type> other_) noexcept {
            if (_STD addressof(other_) != this) {
                storage = _STD move(other_.storage);
            }
            return *this;
        }

        ref<this_type> operator=(nullptr_t) {
            storage.template exchange<nullptr_t>(nullptr);
            return *this;
        }

        ref<this_type> operator=(mref<NonOwningMemoryPointer<Ty_, StorageType_>> other_) = delete;

    public:
        [[nodiscard]] constexpr static bool isAtomic() noexcept {
            return storage_type::isAtomic();
        }

    public:
        template <typename... Args_> requires _STD is_constructible_v<Ty_, Args_...>
        [[nodiscard]] constexpr static this_type make(Args_&&... args_) {
            this_type tmp {};
            tmp.create(_STD forward<Args_>(args_)...);
            return tmp;
        }

    public:
        [[nodiscard]] address_type address() const noexcept {
            return _STD addressof(storage.template load<Ty_>());
        }

    public:
        template <typename Tx_ = Ty_, typename... Args_>
        ref<this_type> create(Args_... args_) {
            storage.allocate(_STD forward<Args_>(args_)...);
            return *this;
        }

        template <typename Tx_ = Ty_> requires
            (not _STD is_const_v<Ty_>) && (not _STD is_void_v<Tx_>) && _STD is_same_v<Tx_, Ty_> &&
            _STD is_default_constructible_v<Tx_>
        ref<Tx_> getOrCreate() noexcept(_STD is_nothrow_default_constructible_v<Tx_>) {
            return (*storage.template loadOrAllocate<Tx_>());
        }

        ref<this_type> destroy() {
            storage.deallocate();
            return *this;
        }

    public:
        template <typename Tx_ = Ty_> requires (not _STD is_const_v<Ty_>)
        ref<this_type> store(Tx_&& value_) noexcept {
            storage.template store<Tx_>(_STD forward<Tx_>(value_));
            return *this;
        }

        template <typename Tx_ = Ty_> requires _STD is_same_v<Tx_, Ty_> && (not similar_to<Ty_, void>)
        cref<_STD remove_const_t<Tx_>> load() const noexcept {
            return *(storage.template load<Tx_>());
        }

        template <typename Tx_ = Ty_> requires _STD is_same_v<Tx_, Ty_>
            && (not _STD is_const_v<Tx_>)
            && (not similar_to<Ty_, void>)
        ref<Tx_> load() noexcept {
            return *(storage.template load<Tx_>());
        }

        [[nodiscard]] auto get() const noexcept {
            return storage.template load<Ty_>();
        }

        [[nodiscard]] auto get() noexcept {
            return storage.template load<Ty_>();
        }

    public:
        template <typename Ptx_ = Ty_*> requires _STD is_nothrow_convertible_v<_STD remove_cvref_t<Ptx_>, Ty_*>
        [[nodiscard]] _STD _Compressed_pair<allocator_type, Ty_*> exchange(Ptx_&& next_) {
            return _STD _Compressed_pair<allocator_type, Ty_*>(
                _STD _One_then_variadic_args_t {},
                allocator_type {},
                storage.exchange(_STD forward<Ptx_>(next_))
            );
        }

        template <typename Ptx_ = Ty_*> requires _STD is_nothrow_convertible_v<_STD remove_cvref_t<Ptx_>, Ty_*>
        [[nodiscard]] std::pair<bool, Ty_*> compare_exchange(
            Ty_* expect_,
            Ptx_&& next_
        ) {
            return storage.compare_exchange(std::forward<Ty_*>(expect_), std::forward<Ptx_>(next_));
        }

    public:
        template <typename Tx_ = Ty_> requires (not similar_to<Tx_, void>) && _STD is_same_v<Tx_, Ty_>
        [[nodiscard]] cref<_STD remove_const_t<Tx_>> operator*() const noexcept {
            return *(storage.template load<Ty_>());
        }

        template <typename Tx_ = Ty_> requires (not _STD is_const_v<Ty_>) && (not similar_to<Tx_, void>) &&
            _STD is_same_v
            <Tx_, Ty_>
        [[nodiscard]] ref<Tx_> operator*() noexcept {
            return *(storage.template load<Ty_>());
        }

        template <typename Tx_ = Ty_> requires _STD is_same_v<Tx_, Ty_>
        [[nodiscard]] const _STD remove_const_t<Tx_>* const operator->() const noexcept {
            return storage.template load<Ty_>();
        }

        template <typename Tx_ = Ty_> requires (not _STD is_const_v<Ty_>) && _STD is_same_v<Tx_, Ty_>
        [[nodiscard]] Tx_* const operator->() noexcept {
            return storage.template load<Ty_>();
        }

    public:
        [[nodiscard]] bool operator!() const noexcept {
            return storage.template load<Ty_>() == nullptr;
        }

        [[nodiscard]] operator bool() const noexcept {
            return storage.template load<Ty_>() != nullptr;
        }

    public:
        [[nodiscard]] constexpr bool operator==(nullptr_t) const noexcept {
            return storage.template load<Ty_>() == nullptr;
        }

        [[nodiscard]] constexpr bool operator!=(nullptr_t) const noexcept {
            return storage.template load<Ty_>() != nullptr;
        }

        [[nodiscard]] constexpr bool operator==(cref<this_type> other_) const noexcept {
            return storage.template load<Ty_>() == other_.storage.template load<Ty_>();
        }

        [[nodiscard]] constexpr bool operator!=(cref<this_type> other_) const noexcept {
            return storage.template load<Ty_>() != other_.storage.template load<Ty_>();
        }

    public:
        storage_type storage;
    };

    /**/

    template <typename Ty_, typename StorageType_>
    struct NonOwningMemoryPointer {
    public:
        using this_type = NonOwningMemoryPointer<Ty_, StorageType_>;

        using value_type = Ty_;
        using storage_type = StorageType_;

        using address_type = _STD ptrdiff_t;
        using difference_type = _STD ptrdiff_t;

    public:
        constexpr NonOwningMemoryPointer() noexcept :
            storage() {}

        constexpr NonOwningMemoryPointer(nullptr_t) noexcept :
            storage(nullptr) {}

        template <
            typename Tx_ = Ty_,
            typename AllocType_ = typename StorageType_::allocator_type,
            typename StorageTx_ = StorageType_>
        constexpr NonOwningMemoryPointer(cref<MemoryPointer<Tx_, AllocType_, StorageTx_>> mp_) :
            storage(mp_.storage) {}

        template <typename Tx_ = Ty_> requires
            _STD is_nothrow_convertible_v<Tx_*, Ty_*>
        constexpr NonOwningMemoryPointer(Tx_* value_) noexcept :
            storage(value_) {}

        template <
            typename Tx_ = Ty_,
            typename StorageTx_ = typename MemoryPointer<Tx_>::storage_type> requires _STD is_nothrow_convertible_v<Tx_*
            , Ty_*>
        constexpr NonOwningMemoryPointer(cref<NonOwningMemoryPointer<Tx_, StorageTx_>> other_) noexcept :
            storage(other_.storage) {}

        constexpr NonOwningMemoryPointer(const this_type& other_) noexcept :
            storage(other_.storage) {}

        template <typename Tx_ = Ty_> requires _STD is_nothrow_convertible_v<Tx_*, Ty_*>
        constexpr NonOwningMemoryPointer(mref<NonOwningMemoryPointer<Tx_>> other_) noexcept :
            storage(_STD move(other_.storage)) {}

        constexpr NonOwningMemoryPointer(this_type&& other_) noexcept :
            storage(_STD move(other_.storage)) {}

        ~NonOwningMemoryPointer() = default;

    public:
        ref<this_type> operator=(this_type&& other_) noexcept {
            if (_STD addressof(other_) != this) {
                storage = _STD move(other_.storage);
            }
            return *this;
        }

        ref<this_type> operator=(const this_type& other_) noexcept {
            if (_STD addressof(other_) != this) {
                storage = other_.storage;
            }
            return *this;
        }

        template <
            typename Tx_ = Ty_,
            typename AllocType_ = typename StorageType_::allocator_type,
            typename StorageTx_ = StorageType_>
        ref<this_type> operator=(cref<MemoryPointer<Tx_, AllocType_, StorageTx_>> mp_) {
            if (static_cast<const void*>(_STD addressof(mp_)) != this) {
                storage = mp_.storage;
            }
            return *this;
        }

        ref<this_type> operator=(nullptr_t) noexcept {
            storage.template exchange<nullptr_t>(nullptr);
            return *this;
        }

    public:
        [[nodiscard]] constexpr static bool isAtomic() noexcept {
            return storage_type::isAtomic();
        }

    public:
        [[nodiscard]] address_type address() const noexcept {
            return _STD addressof(storage.template load<Ty_>());
        }

    public:
        template <typename Tx_ = Ty_> requires (not _STD is_const_v<Ty_>)
        ref<this_type> store(Tx_&& value_) noexcept {
            storage.template store<Tx_>(_STD forward<Tx_>(value_));
            return *this;
        }

        template <typename Tx_ = Ty_> requires (not _STD is_void_v<Tx_>)
            && _STD is_nothrow_convertible_v<ptr<Ty_>, ptr<Tx_>>
        cref<_STD remove_const_t<Tx_>> load() const noexcept {
            return *(storage.template load<Ty_>());
        }

        template <typename Tx_ = Ty_> requires (not _STD is_const_v<Ty_>)
            && (not _STD is_void_v<Tx_>)
            && _STD is_nothrow_convertible_v<ptr<Ty_>, ptr<Tx_>>
        ref<Tx_> load() noexcept {
            return *(storage.template load<Ty_>());
        }

        [[nodiscard]] auto get() const noexcept {
            return storage.template load<Ty_>();
        }

        [[nodiscard]] auto get() noexcept {
            return storage.template load<Ty_>();
        }

    public:
        template <typename Tx_ = Ty_> requires (not _STD is_void_v<Ty_>) && _STD is_same_v<Tx_, Ty_>
        [[nodiscard]] cref<_STD remove_const_t<Tx_>> operator*() const noexcept {
            return *(storage.template load<Ty_>());
        }

        template <typename Tx_ = Ty_> requires (not _STD is_const_v<Ty_>) && (not _STD is_void_v<Ty_>) && _STD is_same_v
            <Tx_, Ty_>
        [[nodiscard]] ref<Tx_> operator*() noexcept {
            return *(storage.template load<Ty_>());
        }

        template <typename Tx_ = Ty_> requires _STD is_same_v<Tx_, Ty_>
        [[nodiscard]] Tx_* const operator->() const noexcept {
            return storage.template load<Ty_>();
        }

        template <typename Tx_ = Ty_> requires (not _STD is_const_v<Ty_>) && _STD is_same_v<Tx_, Ty_>
        [[nodiscard]] Tx_* const operator->() noexcept {
            return storage.template load<Ty_>();
        }

    public:
        [[nodiscard]] constexpr bool operator!() const noexcept {
            return storage.template load<Ty_>() == nullptr;
        }

        [[nodiscard]] explicit constexpr operator bool() const noexcept {
            return storage.template load<Ty_>() != nullptr;
        }

    public:
        [[nodiscard]] constexpr bool operator==(nullptr_t) const noexcept {
            return storage.template load<Ty_>() == nullptr;
        }

        [[nodiscard]] constexpr bool operator!=(nullptr_t) const noexcept {
            return storage.template load<Ty_>() != nullptr;
        }

        [[nodiscard]] constexpr bool operator==(cref<this_type> other_) const noexcept {
            return storage.template load<Ty_>() == other_.storage.template load<Ty_>();
        }

        [[nodiscard]] constexpr bool operator!=(cref<this_type> other_) const noexcept {
            return storage.template load<Ty_>() != other_.storage.template load<Ty_>();
        }

    public:
        storage_type storage;
    };
}

namespace std {
    template <class Type_>
    struct hash<::hg::NonOwningMemoryPointer<Type_>> :
        public ::std::hash<typename ::hg::NonOwningMemoryPointer<Type_>::storage_type::mem_type> {
        [[nodiscard]] size_t operator()(const ::hg::NonOwningMemoryPointer<Type_>& value_) const noexcept {
            return static_cast<
                const _STD hash<typename::hg::NonOwningMemoryPointer<Type_>::storage_type::mem_type>&>(*this)(
                value_.get()
            );
        }
    };
}

/**/
/**/
/**/

namespace hg {
    template <typename Ty>
    using mpt = MemoryPointer<Ty>;

    template <typename Ty>
    using nmpt = typename mpt<Ty>::non_owning_type;
}
