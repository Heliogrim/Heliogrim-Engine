#pragma once

#ifdef _DEBUG
#include <cassert>
#endif

#include <atomic>
#include <concepts>
#include <type_traits>

#include "../__macro.hpp"
#include "../Wrapper.hpp"
#include "../Cast.hpp"

namespace hg {
    namespace {
        using packed_type = _STD uintptr_t;
        using atomic_packed_type = _STD atomic_uintptr_t;

        constexpr packed_type packed_shift = 16ui64;
        constexpr packed_type packed_ref_mask =
            0b00000000'00000000'00000000'00000000'00000000'00000000'11111111'11111111;
        constexpr packed_type packed_ptr_mask =
            0b11111111'11111111'11111111'11111111'11111111'11111111'00000000'00000000;
    };

    template <typename PayloadType_>
    class SharedMemoryReferenceCtrlBlock;

    template <typename PayloadType_>
    class SharedMemoryReference;

    /**/

    template <class Fty_, class Tty_>
    concept IsSmrBiPointerCompatible = _STD is_convertible_v<ptr<Fty_>, ptr<Tty_>> || _STD derived_from<Tty_, Fty_>;

    template <class Fty_, class Tty_>
    concept IsSmrPointerCompatible = _STD is_convertible_v<ptr<Fty_>, ptr<Tty_>>;

    /**/

    template <typename PayloadType_>
    class SharedMemoryReference final {
    public:
        template <typename Ux_>
        friend class SharedMemoryReference;

    public:
        using this_type = SharedMemoryReference<PayloadType_>;
        using value_type = _STD remove_reference_t<PayloadType_>;

        using ctrl_block_type = SharedMemoryReferenceCtrlBlock<value_type>;

    public:
        constexpr SharedMemoryReference() noexcept :
            _ctrlBlock(nullptr),
            _packed(0) {}

        constexpr SharedMemoryReference(
            _In_ const non_owning_rptr<ctrl_block_type> ctrlBlock_,
            _In_ const packed_type packed_
        ) noexcept :
            _ctrlBlock(ctrlBlock_),
            _packed(packed_) {}

        SharedMemoryReference(_In_ cref<this_type> other_) :
            SharedMemoryReference() {

            if (not other_.empty()) {
                *this = _STD move(other_._ctrlBlock->acq());
            }
        }

        SharedMemoryReference(_Inout_ mref<this_type> other_) noexcept :
            _ctrlBlock(_STD exchange(other_._ctrlBlock, nullptr)),
            _packed(_STD exchange(other_._packed, 0)) {}

        template <class Fty_> requires IsSmrPointerCompatible<Fty_, PayloadType_>
        SharedMemoryReference(_Inout_ mref<SharedMemoryReference<Fty_>> other_) noexcept :
            _ctrlBlock(_void_cast<ctrl_block_type>(_STD exchange(other_._ctrlBlock, nullptr))),
            _packed(_STD exchange(other_._packed, 0)) {}

    protected:
        template <class Fty_> requires IsSmrBiPointerCompatible<Fty_, PayloadType_> &&
            (not IsSmrPointerCompatible<Fty_, PayloadType_>)
        explicit SharedMemoryReference(_Inout_ mref<SharedMemoryReference<Fty_>> other_) noexcept :
            _ctrlBlock(_void_cast<ctrl_block_type>(_STD exchange(other_._ctrlBlock, nullptr))),
            _packed(_STD exchange(other_._packed, 0)) {}

    public:
        ~SharedMemoryReference() {
            if (_ctrlBlock != nullptr) {
                _ctrlBlock->rel();
            }
        }

    public:
        ref<this_type> operator=(cref<this_type>) = delete;

        ref<this_type> operator=(_Inout_ mref<this_type> other_) noexcept {
            if (_STD addressof(other_) != this) {
                _STD swap(_ctrlBlock, other_._ctrlBlock);
                _STD swap(_packed, other_._packed);
            }
            return *this;
        }

        template <class Fty_> requires IsSmrPointerCompatible<Fty_, PayloadType_>
        ref<this_type> operator=(_Inout_ mref<SharedMemoryReference<Fty_>> other_) noexcept {
            if (_STD addressof(other_) != this) {
                reset();
                _ctrlBlock = _void_cast<ctrl_block_type>(_STD exchange(other_._ctrlBlock, nullptr));
                _packed = _STD exchange(other_._packed, 0);
            }
            return *this;
        }

    public:
        void swap(ref<this_type> other_) noexcept {

            if (_STD addressof(other_) == this) {
                return;
            }

            _STD swap(_ctrlBlock, other_._ctrlBlock);
            _STD swap(_packed, other_._packed);
        }

    public:
        template <class Fty_, template <typename> typename Pty_> requires (
            IsSmrPointerCompatible<Fty_, PayloadType_> || IsSmrPointerCompatible<PayloadType_, Fty_>
        )
        [[nodiscard]] Pty_<Fty_> into();

        template <class Fty_> requires IsSmrBiPointerCompatible<PayloadType_, Fty_>
        [[nodiscard]] smr<Fty_> into() {
            return smr<Fty_> { _STD move(*this) };
        }

    private:
        ptr<ctrl_block_type> _ctrlBlock;
        packed_type _packed;

    public:
        /**
         * Will return the snapshot reference count
         *
         * @author Julius
         * @date 15.11.2021
         *
         * @returns The counts of references.
         */
        [[nodiscard]] _STD uint16_t refs() const noexcept {
            return _packed;
        }

        /**
         * Will get the stored pointer
         *
         * @author Julius
         * @date 15.11.2021
         *
         * @returns The stored pointer or nullptr.
         */
        [[nodiscard]] non_owning_rptr<value_type> get() const noexcept {
            return reinterpret_cast<non_owning_rptr<value_type>>(_packed >> packed_shift);
        }

        /**
         * Check whether this atomic ctrl ptr has a stored state
         *
         * @author Julius
         * @date 15.11.2021
         *
         * @returns True is any state is present, otherwise false
         */
        [[nodiscard]] _Success_(return == true) bool empty() const noexcept {
            return _ctrlBlock == nullptr;
        }

    public:
        [[nodiscard]] bool operator!() const noexcept {
            return empty();
        }

        [[nodiscard]] operator bool() const noexcept {
            return _ctrlBlock != nullptr;
        }

    public:
        template <typename Ty_ = value_type> requires (!_STD is_void_v<Ty_>) && (!_STD is_array_v<Ty_>)
        [[nodiscard]] ref<Ty_> operator*() const noexcept {
            return *get();
        }

        [[nodiscard]] non_owning_rptr<value_type> operator->() const noexcept {
            return get();
        }

    public:
        void reset() {
            if (_ctrlBlock != nullptr) {
                _ctrlBlock->rel();
            }

            _ctrlBlock = nullptr;
            _packed = 0;
        }
    };

    /**/

    template <typename PayloadType_>
    class SharedMemoryReferenceCtrlBlock final {
    public:
        using this_type = SharedMemoryReferenceCtrlBlock<PayloadType_>;
        using value_type = _STD remove_reference_t<PayloadType_>;

        using smr_type = SharedMemoryReference<value_type>;

    public:
        constexpr SharedMemoryReferenceCtrlBlock() noexcept :
            _packed(0) {}

        SharedMemoryReferenceCtrlBlock(_In_ mref<ptr<value_type>> payload_) :
            _packed(0) {
            this_type::store(_STD move(payload_));
        }

        SharedMemoryReferenceCtrlBlock(cref<this_type>) = delete;

        SharedMemoryReferenceCtrlBlock(mref<this_type>) noexcept = delete;

        ~SharedMemoryReferenceCtrlBlock() {
            this_type::tidy();
        }

    private:
        atomic_packed_type _packed;

    private:
        /**
         * Tidy this up and release stored resource if present
         *
         * @author Julius
         * @date 15.11.2021
         */
        void tidy() {

            const auto packed = _packed.load(_STD memory_order_relaxed);

            /**/
            auto* ctrlp = reinterpret_cast<ptr<value_type>>(packed >> packed_shift);
            if (ctrlp == nullptr) {
                return;
            }

            // TODO: Check whether we want to pay the compile time here, while this is nearly normal behaviour ?!?
            // IM_CORE_WARN("Destructing atomic ctrl block with active store resource.");

            /**/
            this_type::destroy(_STD move(ctrlp));
        }

        void delete_this() const {
            delete this;
        }

        void destroy(mref<ptr<value_type>> obj_) {
            delete obj_;
        }

    public:
        /**
         * Acquire a SharedMemoryReference from this ctrl block
         *
         * @author Julius
         * @date 15.11.2021
         *
         * @returns A atomic ctrl ptr with a possible internal state
         */
        [[nodiscard]] smr_type acq() {

            auto packed = _packed.fetch_add(1, _STD memory_order_release);

            /**/
            if ((packed >> packed_shift) == 0) {
                rel();
                return smr_type {};
            }

            /**/
            return smr_type { this, packed };
        }

        /**
         * Will decrement the reference count and release the internal stored resource if necessary
         *
         * @author Julius
         * @date 15.11.2021
         */
        void rel() {

            const auto packed = _packed.fetch_sub(1, _STD memory_order_release);

            /**
             * Check if this was last reference
             */
            if ((packed & packed_ref_mask) == 0x1) {

                auto expect = _packed.fetch_and(packed_ref_mask, _STD memory_order_release);
                const auto maskedPtr = expect & packed_ptr_mask;

                /**
                 * Check whether another execution acquired reference in the mean time
                 */
                if ((expect & packed_ref_mask) != 0) {
                    /**
                     * Loop fallback strategy
                     */

                    /**
                     * Restore packed state and release responsibility from this execution to other acquirer if present
                     *
                     * Releasing responsibility for releasing underlying resource will prevent starvation problem, if other execution suspend with acquired resource
                     */
                    while (
                        (expect & packed_ref_mask) != 0 &&
                        !_packed.compare_exchange_weak(
                            expect,
                            (expect & packed_ref_mask) | maskedPtr,
                            _STD memory_order_release,
                            _STD memory_order_relaxed
                        )
                    ) { }

                }

                /**
                 * Check whether this is last reference ( or fallback took place )
                 */
                // if (expect & packed_ref_mask)
                if ((expect & packed_ref_mask) == 0 && maskedPtr) {
                    auto* ctrlp = reinterpret_cast<ptr<value_type>>(packed >> packed_shift);
                    this_type::destroy(_STD move(ctrlp));

                    delete_this();
                }
            }
        }

        /**
         * Will store the pointer internally
         *
         * @author Julius
         * @date 15.11.2021
         *
         * @param ptr_ The pointer to store into this ctrl block
         */
        void store(_In_ const ptr<value_type> ptr_) {
            const auto original = reinterpret_cast<packed_type>(ptr_);
            const auto packed = original << packed_shift;

            DEBUG_ASSERT(original == (packed >> packed_shift), "Exceeded addressable memory of shifted memory address.")

            _packed.fetch_and(packed_ref_mask, _STD memory_order_release);
            _packed.fetch_or(packed, _STD memory_order_release);
        }

        /**
         * Will pre-increment the reference counter and then store the pointer internally
         *
         * @author Julius
         * @date 15.11.2021
         *
         * @param ptr_ The pointer to store into the ctrl block
         */
        void push(_In_ const ptr<value_type> ptr_) {
            ++_packed;
            store(ptr_);
        }

        /**
         * Will try to store pointer internally and bumping reference count atomically
         *
         * @author Julius
         * @date 15.11.2021
         *
         * @param ptr_ The pointer to store into the ctrl block
         *
         * @returns True if succeeded, otherwise false
         */
        [[nodiscard]] bool try_push(_In_ const ptr<value_type> ptr_) {

            const auto original = reinterpret_cast<packed_type>(ptr_);
            const auto packed = (original << packed_shift) | 0x1;

            DEBUG_ASSERT(original == (packed >> packed_shift), "Exceeded adressable memory of shifted memory address.")

            packed_type expect { 0 };
            return _packed.compare_exchange_strong(
                expect,
                packed,
                _STD memory_order_release,
                _STD memory_order_relaxed
            );
        }
    };
}

namespace std {
    template <class Type_>
    struct hash<::hg::SharedMemoryReference<Type_>> :
        public ::std::hash<typename ::hg::SharedMemoryReference<Type_>::value_type*> {
        [[nodiscard]] size_t operator()(const ::hg::SharedMemoryReference<Type_>& value_) const noexcept {
            return static_cast<const ::std::hash<typename ::hg::SharedMemoryReference<Type_>::value_type*>&>(*this)(
                value_.get()
            );
        }
    };
}
