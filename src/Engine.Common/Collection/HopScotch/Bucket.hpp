#pragma once

#include <cassert>

#include "Hash.hpp"
#include "../../Types.hpp"
#include "../../__macro.hpp"

namespace hg::hopscotch {
    template <typename ValueType_, u8 NeighborhoodSize_, bool StoreHash_>
    class hopscotch_bucket :
        public hopscotch_bucket_hash<StoreHash_> {
    private:
        constexpr static _STD size_t min_neighborhood_size = 4;
        constexpr static _STD size_t reserved_neighborhood_bits = 2;
        constexpr static _STD size_t max_neighborhood_size = StoreHash_ ?
                                                                 (sizeof(u32) * 8 - reserved_neighborhood_bits) :
                                                                 (sizeof(u64) * 8 - reserved_neighborhood_bits);

        using bucket_hash = hopscotch_bucket_hash<StoreHash_>;

    public:
        using this_type = hopscotch_bucket<ValueType_, NeighborhoodSize_, StoreHash_>;
        using reference_this_type = ref<this_type>;
        using const_reference_this_type = cref<this_type>;

        using value_type = ValueType_;
        using neighbor_bitmap = uint_least8_t;

        using storage_type = _STD aligned_storage_t<sizeof(value_type), alignof(value_type)>;

    public:
        /**
         * Default constructor
         *
         * @author Julius
         * @date 04.10.2021
         */
        hopscotch_bucket() noexcept :
            bucket_hash(),
            _neighbor(0) {}

        /**
         * Copy Constructor
         *
         * @author Julius
         * @date 04.10.2021
         *
         * @param  other_ The other.
         */
        hopscotch_bucket(const_reference_this_type other_) noexcept (_STD is_nothrow_copy_constructible_v<value_type>) :
            bucket_hash(other_),
            _neighbor(other_._neighbor) {

            if (!other_.empty()) {
                ::new(static_cast<ptr<void>>(_STD addressof(_storage))) value_type(other_.value());
            }
        }

        /**
         * Move Constructor
         *
         * @author Julius
         * @date 04.10.2021
         *
         * @param  other_ The other.
         */
        hopscotch_bucket(mref<this_type> other_) noexcept (_STD is_nothrow_move_constructible_v<value_type>) :
            bucket_hash(_STD move(other_)),
            _neighbor(other_._neighbor) {

            if (!other_.empty()) {
                ::new(static_cast<ptr<void>>(_STD addressof(_storage))) value_type(_STD move(other_.value()));
            }
        }

        /**
         * Destructor
         *
         * @author Julius
         * @date 04.10.2021
         */
        ~hopscotch_bucket() noexcept(_STD is_nothrow_destructible_v<value_type>) {
            if (!empty()) {
                destroy();
            }
        }

    public:
        /**
         * Copy Assignment operator
         *
         * @author Julius
         * @date 04.10.2021
         *
         * @param  other_ The other.
         *
         * @returns A shallow copy of this.
         */
        reference_this_type operator=(
            const_reference_this_type other_
        ) noexcept(_STD is_nothrow_copy_constructible_v<value_type>) {

            if (this != _STD addressof(other_)) {
                remove();

                bucket_hash::operator=(other_);
                if (!other_.empty()) {
                    ::new(static_cast<ptr<void>>(_STD addressof(_storage))) value_type(other_.value());
                }
                _neighbor = other_._neighbor;
            }

            return *this;
        }

        /**
         * Move Assignment operator
         *
         * @author Julius
         * @date 04.10.2021
         *
         * @returns A shallow copy of this.
         */
        reference_this_type operator=(mref<this_type>) = delete;

    private:
        /**
         *
         */
        neighbor_bitmap _neighbor;

    public:
        /**
         * Check if bucket is empty
         *
         * @author Julius
         * @date 04.10.2021
         *
         * @returns True if it succeeds, false if it fails.
         */
        [[nodiscard]] bool empty() const noexcept {
            return (_neighbor & 1) == 0;
        }

        /**
         * Check if bucket has overflow
         *
         * @author Julius
         * @date 04.10.2021
         *
         * @returns True if it succeeds, false if it fails.
         */
        [[nodiscard]] bool overflow() const noexcept {
            return (_neighbor & 2) != 0;
        }

        /**
         * Check if neighbor is present by shifted bit
         *
         * @author Julius
         * @date 04.10.2021
         *
         * @param  shift_ The shift.
         *
         * @returns True if it succeeds, false if it fails.
         */
        [[nodiscard]] bool check_neighbor_shift(u8 shift_) const noexcept {
            DEBUG_ASSERT(shift_ <= NeighborhoodSize_, "Check if 'shift_' is present in neighborhood.")
            return ((_neighbor >> (shift_ + reserved_neighborhood_bits)) & 1) == 1;
        }

    public:
        /**
         * Gets the neighbor bitmap
         *
         * @author Julius
         * @date 04.10.2021
         *
         * @returns A neighbor_bitmap.
         */
        [[nodiscard]] neighbor_bitmap neighbor() const noexcept {
            return _neighbor;
        }

    private:
        /**
         * Sets whether this bucket is empty
         *
         * @author Julius
         * @date 04.10.2021
         *
         * @param  empty_ True to empty.
         */
        void set_empty(const bool empty_) noexcept {
            if (empty_) {
                _neighbor = neighbor_bitmap(_neighbor & ~1);
            } else {
                _neighbor = neighbor_bitmap(_neighbor | 1);
            }
        }

    public:
        /**
         * Sets whether bucket has overflow
         *
         * @author Julius
         * @date 04.10.2021
         *
         * @param  overflow_ True to overflow.
         */
        void set_overflow(const bool overflow_) noexcept {
            if (overflow_) {
                _neighbor = neighbor_bitmap(_neighbor | 2);
            } else {
                _neighbor = neighbor_bitmap(_neighbor & ~2);
            }
        }

        /**
         * Set bit in neighbor bitmap by shift
         *
         * @author Julius
         * @date 04.10.2021
         *
         * @param  shift_ The shift.
         */
        void store_neighbor_shift(u8 shift_) noexcept {
            DEBUG_ASSERT(shift_ <= NeighborhoodSize_, "'shift_' should be in neighborhood bitmap range.")
            _neighbor = neighbor_bitmap(_neighbor ^ (1uLL << (shift_ + reserved_neighborhood_bits)));
        }

    private:
        /**
         *
         */
        storage_type _storage;

    public:
        /**
         * Gets the stored value
         *
         * @author Julius
         * @date 04.10.2021
         *
         * @returns A cref&lt;value_type&gt;
         */
        [[nodiscard]] cref<value_type> value() const noexcept {
            DEBUG_ASSERT(!empty(), "Tried to dereference a empty bucket.")
            return *reinterpret_cast<ptr<const value_type>>(_STD addressof(_storage));
        }

        /**
         * Gets the stored value
         *
         * @author Julius
         * @date 04.10.2021
         *
         * @returns A ref&lt;value_type&gt;
         */
        [[nodiscard]] ref<value_type> value() noexcept {
            DEBUG_ASSERT(!empty(), "Tried to dereference a empty bucket.")
            return *reinterpret_cast<ptr<value_type>>(_STD addressof(_storage));
        }

    public:
        /**
         * Stores a value and related hash
         *
         * @author Julius
         * @date 04.10.2021
         *
         * @param  hash_ The hash.
         * @param  value_ The value.
         */
        void store(
            hash_truncate_type hash_,
            mref<value_type> value_
        ) noexcept(
            _STD is_nothrow_move_constructible_v<value_type>
        ) {

            ::new(static_cast<ptr<void>>(_STD addressof(_storage))) value_type(_STD move(value_));
            set_empty(false);
            bucket_hash::set_hash(hash_);
        }

        /**
         * Removes stored value from this bucket and set empty flag
         *
         * @author Julius
         * @date 04.10.2021
         */
        void remove() noexcept(_STD is_nothrow_destructible_v<value_type>) {

            if (!empty()) {
                destroy();
                set_empty(true);
            }
        }

        /**
         * Clears this to its blank/initial state
         *
         * @author Julius
         * @date 04.10.2021
         */
        void clear() noexcept(_STD is_nothrow_destructible_v<value_type>) {

            if (!empty()) {
                destroy();
            }

            _neighbor = 0;
        }

    private:
        /**
         * Destroys the stored value
         *
         * @author Julius
         * @date 04.10.2021
         */
        void destroy() noexcept(_STD is_nothrow_destructible_v<value_type>) {
            DEBUG_ASSERT(!empty(), "Bucket should not be empty while destroy value.")
            value().~value_type();
        }

    public:
        /**
         * Swap stored value into empty bucket
         *
         * @author Julius
         * @date 04.10.2021
         *
         * @param [in,out] other_ The other.
         */
        void swap_into_empty(reference_this_type other_) noexcept(_STD is_move_constructible_v<value_type>) {
            DEBUG_ASSERT(other_.empty(), "Other bucket should be empty to swap value.")

            if (!empty()) {

                ::new(static_cast<ptr<void>>(_STD addressof(other_._storage))) value_type(_STD move(value()));

                other_.copy_hash(*this);
                other_.set_empty(false);

                destroy();
                set_empty(true);
            }
        }
    };
}
