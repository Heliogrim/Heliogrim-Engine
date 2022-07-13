#pragma once

#include <atomic>
#include <cassert>
#include <Engine.Common/stdafx.h>
#include <Engine.Common/Wrapper.hpp>

namespace ember::engine::scheduler {

    /**
     * Enforce atomic resource with release function to erase specular `delete` problem
     */
    template <typename PayloadType_>
    concept IsAtomicPayload = requires(PayloadType_ obj_) {
        { obj_.release() } -> std::same_as<void>;
    } || requires(ptr<PayloadType_> ptr_) {
        { PayloadType_::release(ptr_) } -> std::same_as<void>;
    };

    template <typename PayloadType_>
    concept IsAtomicPayloadStatic = requires(ptr<PayloadType_> ptr_) {
        { PayloadType_::release(ptr_) } -> std::same_as<void>;
    };

    constexpr uint64_t packed_shift = 16ui64;
    constexpr uint64_t packed_ref_mask = 0b00000000'00000000'00000000'00000000'00000000'00000000'11111111'11111111;
    constexpr uint64_t packed_ptr_mask = 0b11111111'11111111'11111111'11111111'11111111'11111111'00000000'00000000;

    template <IsAtomicPayload PayloadType_>
    class AtomicCtrlBlock;

    template <IsAtomicPayload PayloadType_>
    class AtomicCtrlPtr {
    public:
        using value_type = ptr<PayloadType_>;
        using this_type = AtomicCtrlPtr<PayloadType_>;

    public:
        /**
         * Default Constructor
         *
         * @author Julius
         * @date 15.11.2021
         */
        AtomicCtrlPtr() noexcept :
            _ctrlBlock(nullptr),
            _packed(0) {}

        /**
         * Constructor
         *
         * @author Julius
         * @date 15.11.2021
         *
         * @param ctrlBlock_ The control block associated with this ctrl ptr
         * @param packed_ The packed state to get the stored reference count and pointer
         */
        AtomicCtrlPtr(_In_ const ptr<AtomicCtrlBlock<PayloadType_>> ctrlBlock_, _In_ const uint64_t packed_) noexcept :
            _ctrlBlock(ctrlBlock_),
            _packed(packed_) {}

        /**
         * Copy Constructor
         *
         * @author Julius
         * @date 15.11.2021
         */
        AtomicCtrlPtr(cref<this_type>) = delete;

        /**
         * Move Constructor
         *
         * @author Julius
         * @date 15.11.2021
         *
         * @param other_ The other atomic ctrl ptr to move from.
         */
        AtomicCtrlPtr(_Inout_ mref<this_type> other_) noexcept :
            _ctrlBlock(_STD exchange(other_._ctrlBlock, nullptr)),
            _packed(_STD exchange(other_._packed, 0)) {}

        /**
         * Destructor
         *
         * @author Julius
         * @date 15.11.2021
         */
        ~AtomicCtrlPtr() {
            if (_ctrlBlock != nullptr) {
                _ctrlBlock->rel();
            }
        }

    public:
        /**
         * Copy Assignment Operator
         *
         * @author Julius
         * @date 15.11.2021
         */
        ref<AtomicCtrlPtr> operator=(cref<this_type>) = delete;

        /**
         * Move Assignment Operator
         *
         * @author Julius
         * @date 15.11.2021
         *
         * @param other_ The other atomic ctrl ptr to move from.
         *
         * @returns A shallow copy of this
         */
        ref<AtomicCtrlPtr> operator=(_Inout_ mref<this_type> other_) noexcept {
            if (_STD addressof(other_) != this) {
                _STD swap(_ctrlBlock, other_._ctrlBlock);
                _STD swap(_packed, other_._packed);
            }

            return *this;
        }

    private:
        ptr<AtomicCtrlBlock<PayloadType_>> _ctrlBlock;
        _STD uint64_t _packed;

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
         * Will unwrap the stored pointer
         *
         * @author Julius
         * @date 15.11.2021
         *
         * @returns The stored pointer or nullptr.
         */
        [[nodiscard]] value_type unwrap() const noexcept {
            return reinterpret_cast<value_type>(_packed >> packed_shift);
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
        [[nodiscard]] value_type operator*() const noexcept {
            return unwrap();
        }

        [[nodiscard]] value_type operator->() const noexcept {
            return unwrap();
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

    template <IsAtomicPayload PayloadType_>
    class AtomicCtrlBlock {
    public:
        using atomic_ctrl_ptr = AtomicCtrlPtr<PayloadType_>;
        using value_type = typename atomic_ctrl_ptr::value_type;

    public:
        /**
         * Default Constructor
         *
         * @author Julius
         * @date 15.11.2021
         */
        AtomicCtrlBlock() noexcept :
            _packed(0) {}

        /**
         * Constructor
         *
         * @author Julius
         * @date 15.11.2021
         *
         * @param ptr_ The pointer to store into this ctrl block
         */
        AtomicCtrlBlock(_In_ const value_type ptr_) noexcept :
            _packed(0) {
            push(ptr_);
        }

        AtomicCtrlBlock(cref<AtomicCtrlBlock>) = delete;

        AtomicCtrlBlock(mref<AtomicCtrlBlock>) noexcept = delete;

        /**
         * Destructor
         *
         * @author Julius
         * @date 15.11.2021
         */
        ~AtomicCtrlBlock() {
            tidy();
        }

    private:
        /**
         * Tidy this up and release stored resource if present
         *
         * @author Julius
         * @date 15.11.2021
         */
        void tidy() {

            auto packed = _packed.load(_STD memory_order_relaxed);

            /**
             *
             */
            auto* ctrlp = reinterpret_cast<value_type>(packed >> packed_shift);
            if (ctrlp == nullptr) {
                return;
            }

            DEBUG_SNMSG(false, "WARN", "Destructing atomic ctrl block with active store resource.")

            /**
             *
             */
            delete ctrlp;
        }

    public:
        /**
         * Acquire a atomic_ctrl_ptr from this ctrl block
         *
         * @author Julius
         * @date 15.11.2021
         *
         * @returns A atomic ctrl ptr with a possible internal state
         */
        [[nodiscard]] atomic_ctrl_ptr acq() {

            auto packed = _packed.fetch_add(1, _STD memory_order_release);

            /**
             *
             */
            if ((packed >> packed_shift) == 0) {
                rel();
                return {};
            }

            /**
             *
             */
            return AtomicCtrlPtr { this, packed };
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
                /**
                 *
                 */
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
                        !_packed.compare_exchange_weak(expect, (expect & packed_ref_mask) | maskedPtr,
                            _STD memory_order_release, _STD memory_order_relaxed)
                    ) { }

                }

                /**
                 * Check whether this is last reference ( or fallback took place )
                 */
                // if (expect & packed_ref_mask)
                if ((expect & packed_ref_mask) == 0 && maskedPtr) {
                    /**
                     *
                     */
                    auto* ctrlp = reinterpret_cast<value_type>(packed >> packed_shift);
                    if constexpr (IsAtomicPayloadStatic<PayloadType_>) {
                        PayloadType_::release(ctrlp);
                    } else {
                        ctrlp->release();
                    }
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
        void store(_In_ const value_type ptr_) {
            const auto original = reinterpret_cast<uint64_t>(ptr_);
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
        void push(_In_ const value_type ptr_) {
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
        [[nodiscard]] bool try_push(_In_ const value_type ptr_) {

            const auto original = reinterpret_cast<uint64_t>(ptr_);
            const auto packed = (original << packed_shift) | 0x1;

            DEBUG_ASSERT(original == (packed >> packed_shift), "Exceeded adressable memory of shifted memory address.")

            uint64_t expect { 0 };
            return _packed.compare_exchange_strong(expect, packed, _STD memory_order_release,
                _STD memory_order_relaxed);
        }

    private:
        _STD atomic_uint64_t _packed;
    };

    template <IsAtomicPayload PayloadType_, uint_fast16_t Size_>
    class AtomicCtrlBlockPage {
    public:
        using atomic_ctrl_block = AtomicCtrlBlock<PayloadType_>;
        using atomic_ctrl_ptr = typename atomic_ctrl_block::atomic_ctrl_ptr;
        using value_type = typename atomic_ctrl_ptr::value_type;

        using size_type = uint_fast16_t;
        inline static constexpr size_type page_size = Size_;

    public:
        /**
         * Default Constructor
         *
         * @author Julius
         * @date 15.11.2021
         */
        AtomicCtrlBlockPage() noexcept :
            _ctrls {},
            _blocks {} {

            /**
             * Cleanup ctrl block indirection
             */
            for (auto i = 0; i < sizeof(_blocks) / sizeof(ptr<atomic_ctrl_block>); ++i) {
                _blocks[i].store(0, _STD memory_order_relaxed);
            }

        }

        AtomicCtrlBlockPage(cref<AtomicCtrlBlockPage>) = delete;

        AtomicCtrlBlockPage(mref<AtomicCtrlBlockPage>) noexcept = delete;

        /**
         * Destructor
         *
         * @author Julius
         * @date 15.11.2021
         */
        ~AtomicCtrlBlockPage() = default;

    public:
        /**
         * Get a atomic ctrl ptr from this by index
         *
         * @author Julius
         * @date 15.11.2021
         *
         * @param idx_ The index to lookup a resource pointer
         *
         * @returns A atomic ctrl ptr associated with the stored state if present
         */
        [[nodiscard]] atomic_ctrl_ptr get(_In_ const uint64_t idx_) {

            DEBUG_ASSERT(idx_ < page_size, "Index out of bound.")

            auto* ctrlb = _blocks[idx_].load(_STD memory_order_consume);

            /**
             *
             */
            if (ctrlb == nullptr) {
                return {};
            }

            /**
             *
             */
            return ctrlb->acq();
        }

        /**
         * Will signal the ctrl to release the resource if unused
         *
         * @author Julius
         * @date 15.11.2021
         *
         * @param idx_ The index to signal the retirement
         */
        void retire(_In_ const uint64_t idx_) {

            DEBUG_ASSERT(idx_ < page_size, "Index out of bound.")

            auto* ctrlb = _blocks[idx_].exchange(nullptr, _STD memory_order_acq_rel);

            /**
             * If there was no ctrl block stored, nothing to do
             */
            if (ctrlb == nullptr) {
                return;
            }

            /**
             * Bump reference rount
             */
            auto ctrlPtr = ctrlb->acq();

            /**
             * Force double release to retire
             */
            ctrlb->rel();

            /**
             * Function scope destruction will cause ctrl ptr to get into resource release
             */
            return;
        }

        /**
         * Stores a resource pointer at a ctrl by given index
         *
         *  Warning: This will override the previous stored pointer if present
         *
         * @author Julius
         * @date 15.11.2021
         *
         * @param idx_ The index where to store the resource pointer
         * @param ptr_ The resource pointer to store
         */
        void store(_In_ const uint64_t idx_, _In_ const value_type ptr_) {

            DEBUG_ASSERT(idx_ < page_size, "Index out of bound.")

            auto* ctrlb = _blocks[idx_].load(_STD memory_order_consume);

            /**
             * Check if ctrl block is empty
             */
            if (ctrlb == nullptr) {
                /**
                 * Get ctrl block to address
                 */
                auto& nb = _ctrls[idx_];

                /**
                 * First store data to ctrl block
                 */
                nb.push(ptr_);

                /**
                 * Mount ctrl block to addressing space
                 */
                _blocks[idx_].store(&nb, _STD memory_order_release);
                return;
            }

            /**
             * Override if ctrl block is occupied
             */
            ctrlb->store(ptr_);
        }

        [[nodiscard]] bool try_store(_In_ const uint64_t idx_, _In_ const value_type ptr_) {

            DEBUG_ASSERT(idx_ < page_size, "Index out of bound.")

            const auto* ctrlb = _blocks[idx_].load(_STD memory_order_consume);

            /**
             * Check if ctrl block is empty
             */
            if (ctrlb == nullptr) {
                /**
                 * Get ctrl block to address
                 */
                auto& nb = _ctrls[idx_];

                /**
                 * First store data to ctrl block
                 */
                if (!nb.try_push(ptr_)) {
                    return false;
                }

                /**
                 * Mount ctrl block to addressing space
                 */
                _blocks[idx_].store(&nb, _STD memory_order_release);
                return true;
            }

            return false;
        }

    private:
        atomic_ctrl_block _ctrls[page_size];
        _STD atomic<ptr<atomic_ctrl_block>> _blocks[page_size];
    };

}
