#pragma once

#include <atomic>
#include <concepts>
#include <type_traits>

#include "../Make.hpp"
#include "../Types.hpp"
#include "../Concurrent/SharedMemoryReference.hpp"
#include "../Wrapper.hpp"

namespace hg {
    namespace {
        enum class MemoryPointerStorageFlags : bool {
            eUninitialized = false,
            eInitialized = true
        };

        template <typename Ty_, bool Atomic_>
        struct MemoryPointerStorage;

        template <typename Ty_>
        struct MemoryPointerStorage<Ty_, true> {
            using this_type = MemoryPointerStorage<Ty_, true>;

            using mem_type = _STD atomic<Ty_*>;

        public:
            constexpr MemoryPointerStorage() noexcept :
                flags(MemoryPointerStorageFlags::eUninitialized),
                mem(nullptr) {}

        public:
            template <typename AllocType_ = _STD allocator<Ty_>, typename... Args_>
            void allocateWith(AllocType_&& allocator_, Args_&&... args_) {

                using allocator_traits = _STD allocator_traits<AllocType_>;

                auto* nextSnapshot = allocator_.allocate(1);
                _STD construct_at<Ty_, Args_...>(nextSnapshot, _STD forward<Args_>(args_...));

                mem.store(nextSnapshot, _STD memory_order_release);
                flags = MemoryPointerStorageFlags::eInitialized;
            }

            template <typename... Args_> requires _STD is_constructible_v<Ty_, Args_...>
            void allocate(Args_&&... args_) {
                using allocator_type = _STD allocator<Ty_>;
                allocateWith<allocator_type, Args_...>(allocator_type {}, _STD forward<Args_>(args_...));
            }

            template <typename AllocType_ = _STD allocator<Ty_>>
            void allocateUninitialized(AllocType_&& allocator_ = AllocType_ {}) {
                auto* nextSnapshot = allocator_.allocate(1);
                mem.store(nextSnapshot, _STD memory_order::release);
            }

            template <typename AllocType_ = _STD allocator<Ty_>>
            void deallocate(AllocType_&& allocator_ = AllocType_ {}) {

                using allocator_traits = _STD allocator_traits<AllocType_>;

                Ty_* expectation = nullptr;
                auto* snapshot = mem.exchange(expectation, _STD memory_order::acq_rel);

                if (flags == MemoryPointerStorageFlags::eInitialized) {
                    allocator_traits::destroy(allocator_, snapshot);
                }

                allocator_.deallocate(snapshot, 1);

                if (mem.compare_exchange_strong(expectation, nullptr, _STD memory_order_acq_rel)) {
                    flags = MemoryPointerStorageFlags::eUninitialized;
                }
            }

        public:
            template <typename... Args_> requires _STD is_constructible_v<Ty_, Args_...>
            void construct(Args_&&... args_) noexcept(_STD is_nothrow_constructible_v<Ty_, Args_...>) {
                new(mem.load(_STD memory_order::seq_cst)) Ty_(_STD forward<Args_>(args_)...);
                flags = MemoryPointerStorageFlags::eInitialized;
            }

            template <typename... Args_> requires _STD is_constructible_v<Ty_, Args_...>
            bool safeConstruct(Args_&&... args_) noexcept(_STD is_nothrow_constructible_v<Ty_, Args_...>) {

                if (flags == MemoryPointerStorageFlags::eInitialized) {
                    return false;
                }

                auto* const snapshot = mem.load(_STD memory_order::consume);
                if (snapshot == nullptr) {
                    return false;
                }

                new(snapshot) Ty_(_STD forward<Args_>(args_)...);
                flags = MemoryPointerStorageFlags::eInitialized;
                return true;
            }

            template <typename Tx_ = Ty_> requires _STD is_move_assignable_v<Ty_> && _STD is_convertible_v<Tx_, Ty_>
            void store(Tx_&& value_) noexcept {
                (*mem.load(_STD memory_order::seq_cst)) = _STD forward<Tx_>(value_);
            }

            template <typename Tx_ = Ty_> requires _STD is_move_assignable_v<Ty_> && _STD is_convertible_v<Tx_, Ty_>
            bool safeStore(Tx_&& value_) noexcept {

                if (flags == MemoryPointerStorageFlags::eUninitialized) {
                    return false;
                }

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

            template <typename Tx_ = Ty_> requires _STD is_destructible_v<Tx_> && _STD is_same_v<Tx_, Ty_>
            void destruct() noexcept(_STD is_nothrow_destructible_v<Tx_>) {
                (*mem.load(_STD memory_order::relaxed)).~Tx_();
                flags = MemoryPointerStorageFlags::eUninitialized;
            }

            template <typename Tx_ = Ty_> requires _STD is_destructible_v<Tx_> && _STD is_same_v<Tx_, Ty_>
            bool safeDestruct() noexcept(_STD is_nothrow_destructible_v<Tx_>) {

                if (flags == MemoryPointerStorageFlags::eUninitialized) {
                    return false;
                }

                auto* const snapshot = mem.load(_STD memory_order::relaxed);
                if (snapshot == nullptr) {
                    return false;
                }

                (*snapshot).~Tx_();
                flags = MemoryPointerStorageFlags::eUninitialized;
                return true;
            }

        public:
            template <typename Tx_ = Ty_> requires _STD is_same_v<Tx_, Ty_> && _STD is_default_constructible_v<Tx_>
            Tx_* const loadOrAllocate() {

                if (mem == nullptr) {
                    allocate();
                } else if (flags == MemoryPointerStorageFlags::eUninitialized) {
                    safeConstruct();
                }

                return load();
            }

        public:
            MemoryPointerStorageFlags flags : 1;
            mem_type mem;
        };

        template <typename Ty_>
        struct MemoryPointerStorage<Ty_, false> {
            using this_type = MemoryPointerStorage<Ty_, false>;

            using mem_type = Ty_*;

        public:
            constexpr MemoryPointerStorage() noexcept :
                flags(MemoryPointerStorageFlags::eUninitialized),
                mem(nullptr) {}

        public:
            template <typename AllocType_ = _STD allocator<Ty_>, typename... Args_>
            void allocateWith(AllocType_&& allocator_, Args_&&... args_) {

                using allocator_traits = _STD allocator_traits<AllocType_>;

                auto* nextSnapshot = allocator_.allocate(1);
                _STD construct_at<Ty_, Args_...>(nextSnapshot, _STD forward<Args_>(args_)...);

                mem = _STD move(nextSnapshot);
                flags = MemoryPointerStorageFlags::eInitialized;
            }

            template <typename... Args_> requires _STD is_constructible_v<Ty_, Args_...>
            void allocate(Args_&&... args_) {
                using allocator_type = _STD allocator<Ty_>;
                allocateWith<allocator_type, Args_...>(allocator_type {}, _STD forward<Args_>(args_)...);
            }

            template <typename AllocType_ = _STD allocator<Ty_>>
            void allocateUninitialized(AllocType_&& allocator_ = AllocType_ {}) {
                auto* nextSnapshot = allocator_.allocate(1);
                mem = _STD move(nextSnapshot);
            }

            template <typename AllocType_ = _STD allocator<Ty_>>
            void deallocate(AllocType_&& allocator_ = AllocType_ {}) {

                using allocator_traits = _STD allocator_traits<AllocType_>;

                auto* snapshot = _STD exchange(mem, nullptr);

                if (flags == MemoryPointerStorageFlags::eInitialized) {
                    allocator_traits::destroy(allocator_, snapshot);
                }

                allocator_.deallocate(snapshot, 1);
                flags = MemoryPointerStorageFlags::eUninitialized;
            }

        public:
            template <typename... Args_> requires _STD is_constructible_v<Ty_, Args_...>
            void construct(Args_&&... args_) noexcept(_STD is_nothrow_constructible_v<Ty_, Args_...>) {
                new(mem) Ty_(_STD forward<Args_>(args_)...);
                flags = MemoryPointerStorageFlags::eInitialized;
            }

            template <typename... Args_> requires _STD is_constructible_v<Ty_, Args_...>
            bool safeConstruct(Args_&&... args_) noexcept(_STD is_nothrow_constructible_v<Ty_, Args_...>) {

                if (mem == nullptr || flags == MemoryPointerStorageFlags::eInitialized) {
                    return false;
                }

                construct<Args_...>(_STD forward<Args_>(args_)...);
                return true;
            }

            template <typename Tx_ = Ty_> requires _STD is_move_assignable_v<Ty_> && _STD is_convertible_v<Tx_, Ty_>
            void store(Tx_&& value_) noexcept {
                (*mem) = _STD forward<Tx_>(value_);
            }

            template <typename Tx_ = Ty_> requires _STD is_move_assignable_v<Ty_> && _STD is_convertible_v<Tx_, Ty_>
            bool safeStore(Tx_&& value_) noexcept {

                if (mem == nullptr || flags == MemoryPointerStorageFlags::eUninitialized) {
                    return false;
                }

                (*mem) = _STD forward<Tx_>(value_);
                return true;
            }

            template <typename Tx_ = Ty_>
            const Tx_* const load() const noexcept {
                return mem;
            }

            template <typename Tx_ = Ty_>
            Tx_* const load() noexcept {
                return mem;
            }

            template <typename Tx_ = Ty_> requires _STD is_destructible_v<Tx_> && _STD is_same_v<Tx_, Ty_>
            void destruct() noexcept(_STD is_nothrow_destructible_v<Tx_>) {
                (*mem).~Tx_();
                flags = MemoryPointerStorageFlags::eUninitialized;
            }

            template <typename Tx_ = Ty_> requires _STD is_destructible_v<Tx_> && _STD is_same_v<Tx_, Ty_>
            bool safeDestruct() noexcept(_STD is_nothrow_destructible_v<Tx_>) {

                if (mem == nullptr || flags == MemoryPointerStorageFlags::eUninitialized) {
                    return false;
                }

                (*mem).~Tx_();
                flags = MemoryPointerStorageFlags::eUninitialized;

                return true;
            }

        public:
            template <typename Tx_ = Ty_> requires _STD is_same_v<Tx_, Ty_> && _STD is_default_constructible_v<Tx_>
            Tx_* const loadOrAllocate() {

                if (mem == nullptr) {
                    allocate();
                } else if (flags == MemoryPointerStorageFlags::eUninitialized) {
                    construct();
                }

                return load();
            }

        public:
            MemoryPointerStorageFlags flags : 1;
            mem_type mem;
        };
    }

    template <typename Ty_, bool Atomic_ = false>
    struct MemoryPointer {
    public:
        using this_type = MemoryPointer<Ty_, Atomic_>;

        using address_type = _STD ptrdiff_t;
        using address_ref_type = _STD conditional_t<Atomic_, _STD atomic_ptrdiff_t, _STD ptrdiff_t>;

        using storage_non_atomic_type = MemoryPointerStorage<Ty_, false>;
        using storage_atomic_type = MemoryPointerStorage<Ty_, true>;

        template <bool MemoryAtomic_>
        using storage_type = _STD conditional_t<MemoryAtomic_, storage_atomic_type, storage_non_atomic_type>;

    public:
        constexpr MemoryPointer() noexcept :
            storage() { }

    public:
        [[nodiscard]] constexpr static bool isAtomic() noexcept {
            return _STD bool_constant<Atomic_>::value;
        }

    public:
        [[nodiscard]] constexpr static this_type make() {
            this_type tmp {};
            tmp.create();
            return tmp;
        }

    public:
        [[nodiscard]] address_type address() const noexcept {
            return _STD addressof(storage.template load<Ty_>());
        }

    public:
        template <typename Tx_ = Ty_, typename... Args_>
        this_type create(Args_... args_) {
            storage.allocate(_STD forward<Args_>(args_)...);
            return *this;
        }

        template <typename Tx_ = Ty_> requires _STD is_same_v<Tx_, Ty_> && _STD is_default_constructible_v<Tx_>
        ref<Tx_> getOrCreate() noexcept(_STD is_nothrow_default_constructible_v<Tx_>) {
            return (*storage.template loadOrAllocate<Tx_>());
        }

        this_type destroy() {
            storage.deallocate();
            return *this;
        }

    public:
        template <typename Tx_ = Ty_>
        ref<this_type> store(Tx_&& value_) noexcept {
            storage.template store<Tx_>(_STD forward<Tx_>(value_));
            return *this;
        }

        template <typename Tx_ = Ty_>
        cref<Tx_> load() const noexcept {
            return *(storage.template load<Ty_>());
        }

        template <typename Tx_ = Ty_>
        ref<Tx_> load() noexcept {
            return *(storage.template load<Tx_>());
        }

    public:
        [[nodiscard]] cref<Ty_> operator*() const noexcept {
            return *(storage.template load<Ty_>());
        }

        [[nodiscard]] ref<Ty_> operator*() noexcept {
            return *(storage.template load<Ty_>());
        }

        [[nodiscard]] const Ty_* const operator->() const noexcept {
            return storage.template load<Ty_>();
        }

        [[nodiscard]] Ty_* const operator->() noexcept {
            return storage.template load<Ty_>();
        }

    public:
        [[nodiscard]] bool operator!() const noexcept {
            return storage.load() == nullptr;
        }

        [[nodiscard]] operator bool() const noexcept {
            return storage.load() != nullptr;
        }

    public:
        storage_type<Atomic_> storage;
    };
}
