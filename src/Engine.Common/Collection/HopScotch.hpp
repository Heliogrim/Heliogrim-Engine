#pragma once
#include "../stdafx.h"

#include <memory>

namespace ember {

    namespace hopscotch {

        /**
         * Concept for GrowthPolicy
         */
        /*
        template<typename Ty>
        concept IsGrowthPolicy = requires(const Ty obj) {
            { obj.bucket_idx(const _STD size_t) } -> std::same_as<_STD size_t>;
            { obj.next_bucket_count() } -> std::same_as<_STD size_t>;
        };
         */

        /**
         * An exponent 2 growth policy.
         *
         * @author Julius
         * @date 29.10.2020
         *
         * @tparam GrowthFactor Type of the growth factor.
         */
        template <_STD size_t GrowthFactor = 2uLL>
        struct exp2_growth_policy {

            /**
             * Constructor
             *
             * @author Julius
             * @date 29.10.2020
             *
             * @param  min_ The minimum.
             */
            explicit exp2_growth_policy(const _STD size_t min_) :
                _mask(fast_ceil2(min_) - 1u) {}

            /**
             * Bucket index
             *
             * @author Julius
             * @date 29.10.2020
             *
             * @param  hash_ The hash.
             *
             * @returns A size_t.
             */
            [[maybe_unused, nodiscard]] FORCE_INLINE _STD size_t bucket_idx(const _STD size_t hash_) const noexcept {
                return hash_ & _mask;
            }

            /**
             * Next bucket count
             *
             * @author Julius
             * @date 29.10.2020
             *
             * @returns A size_t.
             */
            [[maybe_unused, nodiscard]] FORCE_INLINE _STD size_t next_bucket_count() const {
                return (_mask + 1) * GrowthFactor;
            }

            /**
             * Maximum bucket count
             *
             * @author Julius
             * @date 29.10.2020
             *
             * @returns A size_t.
             */
            [[maybe_unused, nodiscard]] FORCE_INLINE _STD size_t max_bucket_count() const {
                return (_STD numeric_limits<_STD size_t>::max() / 2) + 1uLL;
            }

            /**
             * Clears this  to its blank/initial state
             *
             * @author Julius
             * @date 29.10.2020
             */
            void clear() noexcept {
                _mask = 0;
            }

        private:
            _STD size_t _mask;

            /**
             * Fast ceiling to pow of 2 value
             *
             * @author Julius
             * @date 29.10.2020
             *
             * @param  value_ The value.
             *
             * @returns A size_t.
             */
            static _STD size_t fast_ceil2(_STD size_t value_) {
                if (ip2(value_))
                    return value_;

                if (value_ == 0)
                    return 1;

                --value_;
                for (_STD size_t i = 1; i < sizeof(_STD size_t) * CHAR_BIT; i <<= 1) {
                    value_ |= value_ >> i;
                }

                return value_ + 1u;
            }

            /**
             * Check if value_ is power of 2
             *
             * @author Julius
             * @date 29.10.2020
             *
             * @param  value_ The value.
             *
             * @returns True if it succeeds, false if it fails.
             */
            static constexpr bool ip2(const _STD size_t value_) {
                return value_ != 0 && (value_ & (value_ - 1u)) == 0;
            }

            static_assert(ip2(GrowthFactor), "GrowthFactor have to be a power of two");
            static_assert(GrowthFactor >= 2uLL, "GrowthFactor have to be greater or equal to 2");
        };

        /**
         * A log 2 growth policy.
         *
         * @author Julius
         * @date 29.10.2020
         */
        struct log2_growth_policy { };

        /**
         * A sqr 2 growth policy.
         *
         * @author Julius
         * @date 29.10.2020
         */
        struct sqr2_growth_policy { };

        /** Type of the hash truncate */
        using hash_trunc_type = _STD uint_least32_t;

        /** Number of initialize buckets */
        constexpr static _STD size_t init_bucket_count = 0;

        /** The initialize maximum load factor */
        constexpr static float init_max_load_factor = 0.8F;

        /** The initialize minimum load threshold factor to calculate lower limit*/
        constexpr static float init_min_load_threshold_factor = 0.1F;

        /**
         * A hopscotch bucket hashstore.
         *
         * @author Julius
         * @date 29.10.2020
         *
         * @tparam Store Type of the store.
         */
        template <bool Store>
        class hopscotch_bucket_hashstore {
        public:
            /** The hash equal */
            [[nodiscard]] bool hash_equal([[maybe_unused]] const _STD size_t) const noexcept {
                return true;
            }

            /**
             * Hash truncate
             *
             * @author Julius
             * @date 29.10.2020
             *
             * @returns A hash_trunc_type.
             */
            [[nodiscard, maybe_unused]] hash_trunc_type hash_trunc() const noexcept {
                return 0;
            }

        protected:
            /** The copy hash */
            [[maybe_unused]] void copy_hash([[maybe_unused]] const hopscotch_bucket_hashstore&) noexcept { }

            /** The set hash */
            [[maybe_unused]] void set_hash([[maybe_unused]] const hash_trunc_type&) noexcept { }
        };

        /**
         * A hopscotch bucket hashstore.
         *
         * @author Julius
         * @date 29.10.2020
         */
        template <>
        class hopscotch_bucket_hashstore<true> {
        public:
            /**
             * Query if 'hash_' hash equal
             *
             * @author Julius
             * @date 29.10.2020
             *
             * @param  hash_ The hash.
             *
             * @returns True if it succeeds, false if it fails.
             */
            [[nodiscard]] bool hash_equal(const _STD size_t hash_) const noexcept {
                return _hash_trunc == static_cast<hash_trunc_type>(hash_);
            }

            /**
             * Get the truncated hash
             *
             * @author Julius
             * @date 29.10.2020
             *
             * @returns A hash_trunc_type.
             */
            [[nodiscard]] hash_trunc_type hash_trunc() const noexcept {
                return _hash_trunc;
            }

        protected:
            /**
             * Copies the truncated hash from other_
             *
             * @author Julius
             * @date 29.10.2020
             *
             * @param  other_ The other.
             */
            void copy_hash(const hopscotch_bucket_hashstore& other_) noexcept {
                _hash_trunc = other_._hash_trunc;
            }

            /**
             * Stores truncated hash to bucket
             *
             * @author Julius
             * @date 29.10.2020
             *
             * @param  hash_ The hash.
             */
            void set_hash(const hash_trunc_type& hash_) noexcept {
                _hash_trunc = hash_;
            }

        private:
            hash_trunc_type _hash_trunc;
        };

        /**
         * A hopscotch bucket.
         *
         * @author Julius
         * @date 29.10.2020
         *
         * @tparam ValueType Type of the value type.
         * @tparam NeighborhoodSize Type of the neighborhood size.
         * @tparam StoreHash Type of the store hash.
         */
        template <typename ValueType, uint32_t NeighborhoodSize, bool StoreHash>
        class hopscotch_bucket :
            public hopscotch_bucket_hashstore<StoreHash> {
        private:
            constexpr static _STD size_t min_neighborhood_size = 4;
            constexpr static _STD size_t max_neighborhood_size = StoreHash ? 30u : 62u;
            constexpr static _STD size_t reserved_neighborhood_bits = 2;

            using hashstore = hopscotch_bucket_hashstore<StoreHash>;

        public:
            using this_type = hopscotch_bucket<ValueType, NeighborhoodSize, StoreHash>;
            using value_type = ValueType;
            using neighbor_bitmap = uint_least8_t;

            /**
             * Default constructor
             *
             * @author Julius
             * @date 29.10.2020
             */
            hopscotch_bucket() noexcept :
                hashstore(),
                _neighbor(0) {}

            /**
             * Constructor
             *
             * @author Julius
             * @date 29.10.2020
             *
             * @param  other_ The other.
             */
            hopscotch_bucket(const this_type& other_) noexcept(_STD is_nothrow_copy_constructible<value_type>::value) :
                hashstore(static_cast<hashstore&>(other_)),
                _neighbor(other_._neighbor) {

                if (!other_.empty()) {
                    ::new(static_cast<void*>(_STD addressof(_storage))) value_type(other_.value());
                }
            }

            /**
             * Constructor
             *
             * @author Julius
             * @date 29.10.2020
             *
             * @param [in,out] other_ The other.
             */
            hopscotch_bucket(this_type&& other_) noexcept(_STD is_nothrow_move_constructible<value_type>::value) :
                hashstore(_STD move(static_cast<hashstore&>(other_))),
                _neighbor(other_._neighbor) {

                if (!other_.empty()) {
                    ::new(static_cast<void*>(_STD addressof(_storage))) value_type(_STD move(other_.value()));
                }
            }

            /**
             * Destructor
             *
             * @author Julius
             * @date 29.10.2020
             */
            ~hopscotch_bucket() noexcept {
                if (!empty()) {
                    destroy();
                }
            }

            /**
             * Copy Assignment operator
             *
             * @author Julius
             * @date 02.11.2020
             *
             * @param  other_ The other.
             *
             * @returns A shallow copy of this.
             */
            hopscotch_bucket& operator=(
                const hopscotch_bucket& other_) noexcept(_STD is_nothrow_copy_constructible<value_type>::value) {
                if (this != &other_) {
                    remove();

                    hashstore::operator=(static_cast<hashstore&>(other_));
                    if (!other_.empty()) {
                        ::new(static_cast<void*>(_STD addressof(_storage))) value_type(other_._value());
                    }

                    _neighbor = other_._neighbor;
                }

                return *this;
            }

            /**
             * Check if bucket is empty
             *
             * @author Julius
             * @date 29.10.2020
             *
             * @returns True if it succeeds, false if it fails.
             */
            [[nodiscard]] bool empty() const noexcept {
                return (_neighbor & 1) == 0;
            }

            /**
             * Gets the value
             *
             * @author Julius
             * @date 29.10.2020
             *
             * @returns A reference to a value_type.
             */
            [[nodiscard]] value_type& value() noexcept {
                DEBUG_ASSERT(!empty(), "Tried to dereference a empty bucket.")
                return *reinterpret_cast<value_type*>(_STD addressof(_storage));
            }

            /**
             * Gets the value
             *
             * @author Julius
             * @date 29.10.2020
             *
             * @returns A reference to a const value_type.
             */
            [[nodiscard]] const value_type& value() const noexcept {
                DEBUG_ASSERT(!empty(), "Tried to dereference a empty bucket.")
                return *reinterpret_cast<const value_type*>(_STD addressof(_storage));
            }

            /**
             * Stores a value and truncated hash
             *
             * @author Julius
             * @date 02.11.2020
             *
             * @param 		   trunc_hash_ The truncate hash.
             * @param [in,out] value_ The value.
             */
            void store(hash_trunc_type trunc_hash_,
                value_type&& value_) noexcept(_STD is_nothrow_move_constructible<value_type>::value) {
                ::new(static_cast<void*>(_STD addressof(_storage))) value_type(_STD forward<value_type>(value_));
                set_empty(false);
                hashstore::set_hash(trunc_hash_);
            }

            /**
             * Removes value from bucket storage
             *
             * @author Julius
             * @date 02.11.2020
             */
            void remove() noexcept {
                if (!empty()) {
                    destroy();
                    set_empty(true);
                }
            }

            /**
             * Clears this to its blank/initial state
             *
             * @author Julius
             * @date 29.10.2020
             */
            void clear() noexcept {
                if (!empty()) {
                    destroy();
                }

                _neighbor = 0;
                DEBUG_ASSERT(empty(), "Bucket should be empty after clear command.")
            }

            /**
             * Swap into empty
             *
             * @author Julius
             * @date 29.10.2020
             *
             * @param [in,out] other_ The other.
             */
            void swap_into_empty(hopscotch_bucket& other_) {
                DEBUG_ASSERT(other_.empty(), "Other bucket should be empty to swap value.")
                if (!empty()) {
                    ::new(static_cast<void*>(_STD addressof(other_._storage))) value_type(_STD move(value()));

                    other_.copy_hash(*this);
                    other_.set_empty(false);

                    destroy();
                    set_empty(true);
                }
            }

            /**
             * Stores neighbor shift
             *
             * @author Julius
             * @date 29.10.2020
             *
             * @param  shift_ The shift.
             */
            void store_neighbor_shift(uint32_t shift_) noexcept {
                DEBUG_ASSERT(shift_ <= NeighborhoodSize, "'shift_' should be in neighborhood range.")
                _neighbor = static_cast<neighbor_bitmap>(_neighbor ^ (1ull << (shift_ + reserved_neighborhood_bits)));
            }

            /**
             * Check neighbor shift
             *
             * @author Julius
             * @date 29.10.2020
             *
             * @param  shift_ The shift.
             *
             * @returns True if it succeeds, false if it fails.
             */
            [[nodiscard]] bool check_neighbor_shift(uint32_t shift_) const noexcept {
                DEBUG_ASSERT(shift_ <= NeighborhoodSize, "Check if 'shift_' is present in neighborhood.")
                return ((_neighbor >> (shift_ + reserved_neighborhood_bits)) & 1) == 1;
            }

            /**
             * Gets the neighbor
             *
             * @author Julius
             * @date 29.10.2020
             *
             * @returns A neighbor_bitmap.
             */
            [[nodiscard]] neighbor_bitmap neighbor() const {
                return _neighbor;
            }

        private:
            using storage = typename _STD aligned_storage<sizeof(value_type), alignof(value_type)>::type;

            neighbor_bitmap _neighbor;
            storage _storage;

            /**
             * Sets bitmap to empty or not
             *
             * @author Julius
             * @date 29.10.2020
             *
             * @param  empty_ True to empty.
             */
            void set_empty(const bool empty_) noexcept {
                if (empty_)
                    _neighbor = static_cast<neighbor_bitmap>(_neighbor & ~1);
                else
                    _neighbor = static_cast<neighbor_bitmap>(_neighbor | 1);
            }

            /**
             * Destroys this 
             *
             * @author Julius
             * @date 29.10.2020
             */
            void destroy() noexcept {
                DEBUG_ASSERT(!empty(), "Bucket should not be empty to destroy value")
                value().~value_type();
            }
        };

        /**
         * A hopscotch hash.
         *
         * @author Julius
         * @date 29.10.2020
         *
         * @tparam ValueType Type of the value type.
         * @tparam Hash Type of the hash.
         * @tparam KeyEqual Type of the key equal.
         * @tparam Allocator Type of the allocator.
         * @tparam NeighborhoodSize Type of the neighborhood size.
         * @tparam StoreHash Type of the store hash.
         * @tparam GrowthPolicy Type of the growth policy.
         */
        template <
            typename ValueType,
            class Hash,
            class KeyEqual,
            class Allocator,
            uint32_t NeighborhoodSize,
            bool StoreHash,
            class GrowthPolicy
        >
        class hopscotch_hash :
            private Hash,
            private KeyEqual,
            private GrowthPolicy {
        public:
            using value_type = ValueType;

            using this_type = hopscotch_hash<ValueType, Hash, KeyEqual, Allocator, NeighborhoodSize, StoreHash,
                GrowthPolicy>;

            using hash_type = _STD uint64_t;
            using index_type = _STD size_t;
            using size_type = _STD size_t;

            using difference_type = _STD ptrdiff_t;
            using hasher_type = Hash;
            using key_equal = KeyEqual;
            using allocator_type = Allocator;

        private:
            using bucket_type = hopscotch_bucket<ValueType, NeighborhoodSize, StoreHash>;
            using bucket_allocator = typename _STD allocator_traits<allocator_type>::template rebind_alloc<bucket_type>;
            using bucket_container_type = _STD vector<bucket_type>;

            using bucket_iterator = typename bucket_container_type::iterator;
            using bucket_const_iterator = typename bucket_container_type::const_iterator;

        public:
            /**
             * A hopscotch iterator. This class cannot be inherited..
             *
             * @author Julius
             * @date 29.10.2020
             *
             * @tparam Const Type of the constant.
             */
            template <bool Const>
            class hopscotch_iterator final {
                friend class hopscotch_hash;
            protected:
                using bucket_iterator = _STD conditional_t<Const,
                    typename hopscotch_hash::bucket_const_iterator,
                    typename hopscotch_hash::bucket_iterator
                >;

                /**
                 * Constructor
                 *
                 * @author Julius
                 * @date 28.10.2020
                 *
                 * @param  bucket_iterator_ The bucket iterator.
                 * @param  bucket_iterator_end_ The bucket iterator end.
                 */
                hopscotch_iterator(bucket_iterator bucket_iterator_, bucket_iterator bucket_iterator_end_) noexcept :
                    _bucket_iterator(bucket_iterator_),
                    _bucket_iterator_end(bucket_iterator_end_) {}

            public:
                using iterator_category = _STD forward_iterator_tag;
                using value_type = typename _STD conditional<Const, const typename hopscotch_hash::value_type, typename
                    hopscotch_hash::value_type>::type;
                using difference_type = _STD ptrdiff_t;

                using this_type = hopscotch_iterator<Const>;

                /**
                 * Default constructor
                 *
                 * @author Julius
                 * @date 28.10.2020
                 */
                hopscotch_iterator() noexcept = default;

                /**
                 * Copy Constructor
                 *
                 * @author Julius
                 * @date 28.10.2020
                 *
                 * @param  other_ The other.
                 */
                hopscotch_iterator(const hopscotch_iterator& other_) = default;

                /**
                 * Move Constructor
                 *
                 * @author Julius
                 * @date 28.10.2020
                 *
                 * @param [in,out] other_ The other.
                 */
                hopscotch_iterator(hopscotch_iterator&& other_) noexcept = default;

                /**
                 * Copy Assignment operator
                 *
                 * @author Julius
                 * @date 28.10.2020
                 *
                 * @param  other_ The other.
                 *
                 * @returns A shallow copy of this.
                 */
                this_type& operator=(const this_type& other_) = default;

                /**
                 * Move Assignment operator
                 *
                 * @author Julius
                 * @date 28.10.2020
                 *
                 * @param [in,out] other_ The other.
                 *
                 * @returns A shallow copy of this.
                 */
                this_type& operator=(this_type&& other_) noexcept = default;

                /**
                 * Gets the value
                 *
                 * @author Julius
                 * @date 28.10.2020
                 *
                 * @returns A reference to a value_type.
                 */
                value_type& value() const {
                    return _bucket_iterator->value();
                }

                /**
                 * Indirection operator
                 *
                 * @author Julius
                 * @date 28.10.2020
                 *
                 * @returns The result of the operation.
                 */
                value_type& operator*() const {
                    return _bucket_iterator->value();
                }

                /**
                 * Member dereference operator
                 *
                 * @author Julius
                 * @date 28.10.2020
                 *
                 * @returns The dereferenced object.
                 */
                value_type* operator->() const {
                    return _STD addressof(_bucket_iterator->value());
                }

                /**
                 * Increment operator
                 *
                 * @author Julius
                 * @date 28.10.2020
                 *
                 * @returns The result of the operation.
                 */
                this_type& operator++() {
                    if (_bucket_iterator == _bucket_iterator_end) {
                        ++_bucket_iterator;
                    }
                    return *this;
                }

                /**
                 * Increment operator
                 *
                 * @author Julius
                 * @date 28.10.2020
                 *
                 * @returns The result of the operation.
                 */
                this_type operator++(int) {
                    this_type tmp { *this };
                    ++(*this);
                    return tmp;
                }

                /**
                 * Equality operator
                 *
                 * @author Julius
                 * @date 29.10.2020
                 *
                 * @param  left_ The first instance to compare.
                 * @param  right_ The second instance to compare.
                 *
                 * @returns True if the parameters are considered equivalent.
                 */
                friend bool operator==(const this_type& left_, const this_type& right_) {
                    return left_._bucket_iterator == right_._bucket_iterator;
                }

                /**
                 * Inequality operator
                 *
                 * @author Julius
                 * @date 29.10.2020
                 *
                 * @param  left_ The first instance to compare.
                 * @param  right_ The second instance to compare.
                 *
                 * @returns True if the parameters are not considered equivalent.
                 */
                friend bool operator!=(const this_type& left_, const this_type& right_) {
                    return left_._bucket_iterator != right_._bucket_iterator;
                }

            private:
                bucket_iterator _bucket_iterator;
                bucket_iterator _bucket_iterator_end;
            };

            using iterator = hopscotch_iterator<false>;
            using const_iterator = hopscotch_iterator<true>;

        private:
            /**
             * Static empty bucket
             *
             * @author Julius
             * @date 02.11.2020
             *
             * @returns Null if it fails, else a pointer to a bucket_type.
             */
            [[nodiscard]] static bucket_type* static_empty_bucket() noexcept {
                static bucket_type empty_bucket;
                return &empty_bucket;
            }

            /**
             * Hashes the given value
             *
             * @author Julius
             * @date 28.10.2020
             *
             * @param  value_ The value.
             *
             * @returns A size_t.
             */
            [[nodiscard]] FORCE_INLINE _STD size_t hash(const value_type& value_) const {
                return hasher_type::operator()(value_);
            }

            /**
             * Compares two const hash_type&amp; objects to determine their relative ordering
             *
             * @author Julius
             * @date 28.10.2020
             *
             * @param  left_ Constant hash type&amp; to be compared.
             * @param  right_ Constant hash type&amp; to be compared.
             *
             * @returns True if it succeeds, false if it fails.
             */
            [[nodiscard]] FORCE_INLINE bool compare(const hash_type& left_, const hash_type& right_) const {
                return key_equal::operator()(left_, right_);
            }

            /**
             * Use stored hash
             *
             * @author Julius
             * @date 29.10.2020
             *
             * @param  count_ Number of.
             *
             * @returns True if it succeeds, false if it fails.
             */
            [[nodiscard]] static bool use_stored_hash(const size_type count_) {
                /* (count_ - 1) <= _STD numeric_limits<hash_trunc_type>::max() */
                return count_ < _STD numeric_limits<hash_trunc_type>::max();
            }

            /**
             * Updates the load meta relative to factor_
             *
             * @author Julius
             * @date 07.11.2020
             *
             * @param  factor_ The factor.
             */
            void update_load_meta(const float factor_) noexcept {
                _max_load_factor = MAX(0.1F, MIN(factor_, 0.95F));
                _min_load_threshold = static_cast<size_type>(static_cast<float>(bucket_count()) *
                    init_min_load_threshold_factor);
                _max_load_threshold = static_cast<size_type>(static_cast<float>(bucket_count()) * _max_load_factor);
            }

            /**
             * Resolves the bucket index based on the hash with GrowthPolicy
             *
             * @author Julius
             * @date 28.10.2020
             *
             * @param  hash_ The hash.
             *
             * @returns An index_type.
             */
            [[nodiscard]] index_type bucket_idx(const hash_type& hash_) const {
                const index_type idx = GrowthPolicy::bucket_idx(hash_);
                DEBUG_ASSERT(idx < _buckets.size() || (idx == 0 && _buckets.empty()),
                    "'idx' by hash should be a valid bucket.")
                return idx;
            }

            /**
             * Erases a element from bucket
             *
             * @author Julius
             * @date 28.10.2020
             *
             * @param [in,out] bucket_ The bucket.
             * @param 		   idx_ Zero-based index of the.
             */
            void bucket_erase(bucket_type& bucket_, index_type idx_) noexcept {
                const index_type vidx = _STD distance(_buckets.data(), &bucket_);
                DEBUG_ASSERT(vidx >= idx_, "")

                bucket_.remove();
                _buckets[idx_].store_neighbor_shift(vidx - idx_);
                --_element_count;
            }

            /**
             * Bucket insert
             *
             * @author Julius
             * @date 02.11.2020
             *
             * @param 		   eidx_ The eidx.
             * @param 		   tidx_ The tidx.
             * @param 		   hash_ The hash.
             * @param [in,out] value_ The value.
             *
             * @returns A bucket_iterator.
             */
            [[nodiscard]] bucket_iterator bucket_insert(const index_type eidx_, const index_type tidx_,
                const hash_type hash_, value_type&& value_) {
                _buckets_ptr[eidx_].store(static_cast<hash_trunc_type>(hash_), _STD forward<value_type>(value_));
                _buckets_ptr[tidx_].store_neighbor_shift(eidx_ - tidx_);

                ++_element_count;

                return _buckets.begin() + eidx_;
            }

            /**
             * Searches for the first empty bucket
             *
             * @author Julius
             * @date 29.10.2020
             *
             * @param  from_ Source for the.
             *
             * @returns If succeed index of empty bucket or if failed collection size
             */
            [[nodiscard]] FORCE_INLINE index_type find_empty_bucket(index_type from_) const {
                const index_type limit = _STD min(from_ + NeighborhoodSize, _buckets.size());

                for (; from_ < limit; ++from_) {
                    if (_buckets_ptr[from_].empty())
                        return from_;
                }

                return _buckets.size();
            }

            /**
             * Tries to move empty bucket to lower index
             *
             * @author Julius
             * @date 29.10.2020
             *
             * @param [in,out] empty_ The empty.
             *
             * @returns True if it succeeds, false if it fails.
             */
            [[nodiscard]] FORCE_INLINE bool move_empty_bucket_lower(IN OUT index_type& empty_) {
                const index_type start = empty_ - NeighborhoodSize + 1u;
                for (index_type idx = start; idx < empty_; ++idx) {

                    typename bucket_type::neighbor_bitmap bitmap = _buckets_ptr[idx].neighbor();
                    index_type swap_idx = idx;

                    while (bitmap != 0 && swap_idx < empty_) {
                        if ((bitmap & 1) == 1) {

                            _buckets_ptr[swap_idx].swap_into_empty(_buckets_ptr[empty_]);
                            _buckets_ptr[idx].store_neighbor_shift(empty_ - idx);
                            _buckets_ptr[idx].store_neighbor_shift(swap_idx - idx);

                            empty_ = swap_idx;
                            return true;
                        }

                        ++swap_idx;
                        bitmap >>= 1;
                    }
                }

                return false;
            }

            /**
             * Bucket find
             *
             * @author Julius
             * @date 02.11.2020
             *
             * @param 		   value_ The value.
             * @param 		   hash_ The hash.
             * @param [in]	   bucket_ If non-null, the bucket.
             *
             * @returns Null if it fails, else a pointer to a bucket_type.
             */
            [[nodiscard]] bucket_type* bucket_find(
                const value_type& value_,
                const hash_type& hash_,
                IN bucket_type* bucket_
            ) const noexcept {
                typename bucket_type::neighbor_bitmap nbm = bucket_->neighbor();

                /**
                 * Check for value in addresses bucket and neighborhood
                 */
                while (nbm != 0) {
                    /**
                     * If current bit has stored value
                     */
                    if ((nbm & 1) == 1) {
                        /**
                         * Check whether current bucket does contain the queried value
                         */
                        if ((!StoreHash || bucket_->hash_equal(hash_)) && key_equal::operator()(bucket_->value(),
                            value_)) {
                            return bucket_;
                        }
                    }

                    /**
                     * Shift current bucket to next
                     */
                    ++bucket_;

                    /**
                     * Pop first neighbor bit
                     */
                    nbm >>= 1;
                }

                return nullptr;
            }

            /**
             * Bucket find
             *
             * @author Julius
             * @date 02.11.2020
             *
             * @param 		   value_ The value.
             * @param 		   hash_ The hash.
             * @param [in]     bucket_ If non-null, the bucket.
             *
             * @returns Null if it fails, else a pointer to a bucket_type.
             */
            [[nodiscard]] bucket_type* bucket_find(
                const value_type& value_,
                const hash_type& hash_,
                IN bucket_type* bucket_
            ) noexcept {
                const bucket_type* bf = static_cast<const this_type*>(this)->bucket_find(value_, hash_, bucket_);
                return const_cast<bucket_type*>(bf);
            }

            /**
             * Searches for the first implementation
             *
             * @author Julius
             * @date 02.11.2020
             *
             * @param 		   value_ The value.
             * @param 		   hash_ The hash.
             * @param [in,out] bucket_ If non-null, the bucket.
             *
             * @returns The found implementation.
             */
            [[nodiscard]] const_iterator find_impl(
                const value_type& value_,
                const hash_type& hash_,
                bucket_type* bucket_
            ) const noexcept {
                const bucket_type* bf = bucket_find(value_, hash_, bucket_);

                /**
                 * Check if value could be found in bucket or neighborhood
                 */
                if (bf != nullptr) {
                    return const_iterator {
                        _buckets.cbegin() + _STD distance(_buckets.data(), bf),
                        _buckets.cend()
                    };
                }

                /**
                 * If not found, return end iterator of bucket collection type
                 */
                return const_iterator {
                    _buckets.cend(),
                    _buckets.cend()
                };
            }

            /**
             * Searches for the first implementation
             *
             * @author Julius
             * @date 02.11.2020
             *
             * @param 		   value_ The value.
             * @param 		   hash_ The hash.
             * @param [in,out] bucket_ If non-null, the bucket.
             *
             * @returns The found implementation.
             */
            [[nodiscard]] iterator find_impl(
                const value_type& value_,
                const hash_type& hash_,
                bucket_type* bucket_
            ) noexcept {
                bucket_type* bf = bucket_find(value_, hash_, bucket_);

                /**
                 * Check if value could be found in bucket or neighborhood
                 */
                if (bf != nullptr) {
                    return iterator { _buckets.begin() + _STD distance(_buckets.data(), bf), _buckets.end() };
                }

                /**
                 * If not found, return end iterator of bucket collection type
                 */
                return iterator {
                    _buckets.end(),
                    _buckets.end()
                };
            }

            /**
             * Check whether rehash will change neighborhood situation to store current target bucket
             *
             * @author Julius
             * @date 07.11.2020
             *
             * @param  cidx_ The cidx.
             *
             * @returns True if it succeeds, false if it fails.
             */
            bool will_change_neighbor(const index_type cidx_) const noexcept {
                /**
                 * Get Policy constraints for next container size step
                 */
                size_type extended = GrowthPolicy::next_bucket_count();
                GrowthPolicy policy { extended };

                /**
                 * Check whether use stored hashed of buckets
                 */
                const bool stored = use_stored_hash(extended);

                /**
                 * Get upper boundary of neighborhood
                 */
                const index_type upper = cidx_ + NeighborhoodSize;

                /**
                 * Iterate over target bucket up to neighborhood boundary
                 */
                for (index_type idx = cidx_; idx < _buckets.size() && idx < upper; ++idx) {
                    /**
                     * Get hash value of current checking bucket
                     */
                    const hash_type h = stored ? _buckets_ptr[idx].hash_trunc() : hash(_buckets_ptr[idx].value());

                    /**
                     * If bucket of neighborhood will get moved to another bucket
                     */
                    if (bucket_idx(h) != policy.bucket_idx(h)) {
                        return true;
                    }
                }

                /**
                 * Growth Step won't change bucket neighborhood
                 */
                return false;
            }

            /**
             * Inserts a value
             *
             * @author Julius
             * @date 29.10.2020
             *
             * @param 		   idx_ Zero-based index of the.
             * @param 		   hash_ The hash.
             * @param [in,out] value_ The value.
             *
             * @returns A pair&lt;iterator,bool&gt;
             */
            [[nodiscard]] FORCE_INLINE _STD pair<iterator, bool> insert_value(index_type idx_, const hash_type& hash_,
                value_type&& value_) {
                if (_element_count >= _max_load_threshold) {
                    rehash(GrowthPolicy::next_bucket_count());
                    idx_ = bucket_idx(hash_);
                }

                index_type empty_idx = find_empty_bucket(idx_);
                if (empty_idx < _buckets.size()) {
                    do {
                        DEBUG_ASSERT(empty_idx >= idx_, "")

                        /**
                         * If empty bucket is in range of neighborhood
                         */
                        if ((empty_idx - idx_) < NeighborhoodSize) {
                            auto it = bucket_insert(empty_idx, idx_, hash_, _STD forward<value_type>(value_));
                            return _STD make_pair(iterator { it, _buckets.end() }, true);
                        }
                    } while (move_empty_bucket_lower(empty_idx));
                }

                /**
                 * Check whether
                 *	-> current load is overall load or just peak cause of hash collision ( bad hashing )
                 *	-> grow of container will change neighborhood
                 */
                if (size() < _min_load_threshold || !will_change_neighbor(idx_)) {
                    // TODO: Overflow Container if too many hash collisions or to much grow required
                    return _STD make_pair<iterator, bool>({}, false);
                }

                /**
                 * Grow container
                 */
                rehash(GrowthPolicy::next_bucket_count());
                idx_ = bucket_idx(hash_);

                /**
                 * Repeat recursive until insert succeeds
                 */
                return insert_value(idx_, hash_, _STD forward<value_type>(value_));
            }

            /**
             * Implementation of Insert for payload_
             *
             * @tparam PayloadType Type of the payload type.
             * @param [in,out] payload_ The payload.
             *
             * @returns A pair&lt;iterator,bool&gt;
             */
            template <typename PayloadType>
            [[nodiscard]] FORCE_INLINE _STD pair<iterator, bool> insert_impl(PayloadType&& payload_) {
                const hash_type h = hash(payload_);
                const index_type idx = bucket_idx(h);

                /**
                 * Check if already present
                 */
                auto it = find_impl(payload_, h, _buckets_ptr + idx);
                return (it != end()) ?
                           _STD make_pair(it, false) :
                           insert_value(idx, h, _STD forward<PayloadType>(payload_));
            }

            /**
             * Implementation of try emplace for payload_
             *
             * @tparam PayloadType Type of the payload type.
             * @param [in,out] payload_ The payload.
             *
             * @returns A pair&lt;iterator,bool&gt;
             */
            template <typename PayloadType>
            [[nodiscard]] FORCE_INLINE _STD pair<iterator, bool> try_emplace_impl(PayloadType&& payload_) {
                const hash_type h = hash(payload_);
                const index_type idx = bucket_idx(h);

                /**
                 * Check if already exists
                 */
                auto it = find_impl(payload_, h, _buckets_ptr + idx);
                return (it != end()) ?
                           _STD make_pair(it, false) :
                           insert_value(idx, h, _STD forward<PayloadType>(payload_));
            }

            /**
             * Implementation of inserts an or assign for payload_
             *
             * @tparam PayloadType Type of the payload type.
             * @param [in,out] payload_ The payload.
             *
             * @returns A pair&lt;iterator,bool&gt;
             */
            template <typename PayloadType>
            [[nodiscard]] FORCE_INLINE _STD pair<iterator, bool> insert_or_assign_impl(PayloadType&& payload_) {
                auto it = try_emplace_impl(_STD forward<PayloadType>(payload_));
                return it.second ? it : it.first.value() = _STD forward<PayloadType>(payload_);
            }

            /**
             * Rehash implementation
             *
             * @param  count_ Number of requested elements.
             */
            FORCE_INLINE void rehash_impl(const size_type count_) {
                this_type new_hh = deduce(count_);

                try {
                    const bool use_hash = use_stored_hash(new_hh.bucket_count());
                    for (auto bi = _buckets.begin(); bi != _buckets.end(); ++bi) {
                        if (bi->empty())
                            continue;
                        const hash_type h = use_hash ? bi->hash_trunc() : new_hh.hash_function()(bi->value());
                        const index_type idx = new_hh.bucket_idx(h);
                        new_hh.insert_value(idx, h, _STD move(bi->value()));

                        bucket_erase(*bi, bucket_idx(h));
                    }
                } catch (...) {
                    const bool use_hash = use_stored_hash(new_hh.bucket_count());
                    for (auto bi = new_hh._buckets.begin(); bi != new_hh._buckets.end(); ++bi) {
                        if (bi->empty())
                            continue;

                        const hash_type hash = use_hash ? bi->hash_trunc() : hash_function()(bi->value());
                        const index_type idx = bucket_idx(hash);

                        insert_value(idx, hash, _STD move(bi->value()));
                    }

                    throw;
                }

                //new_hh.swap(*this);
                *this = _STD move(new_hh);
            }

            /**
             * Deduces a empty copy of this with manipulated parameters
             *
             * @author Julius
             * @date 28.10.2020
             *
             * @param  buckets_ The buckets.
             *
             * @returns A this_type.
             */
            FORCE_INLINE this_type deduce(size_type buckets_) const {
                return this_type(buckets_, static_cast<const hasher_type&>(*this), static_cast<const key_equal&>(*this),
                    allocator(), _max_load_factor);
            }

        public:
            /**
             * Constructor
             *
             * @author Julius
             * @date 02.11.2020
             *
             * @param  buckets_ The buckets.
             * @param  hash_ The hash.
             * @param  key_equal_ The key equal.
             * @param  allocator_ The allocator.
             * @param  max_load_factor_ The maximum load factor.
             */
            hopscotch_hash(size_type buckets_, const hasher_type& hash_, const key_equal& key_equal_,
                const allocator_type& allocator_, float max_load_factor_) :
                Hash(hash_),
                KeyEqual(key_equal_),
                GrowthPolicy(buckets_),
                _buckets(allocator_),
                _buckets_ptr(this_type::static_empty_bucket()),
                _max_load_factor(max_load_factor_),
                _element_count(0) {

                if (buckets_ > 0) {
                    _buckets.resize(buckets_ + NeighborhoodSize - 1u);
                    _buckets_ptr = _buckets.data();
                }

                update_load_meta(max_load_factor_);
            }

            /**
             * Copy Constructor
             *
             * @author Julius
             * @date 28.10.2020
             *
             * @param  other_ The other.
             */
            hopscotch_hash(const this_type& other_) :
                Hash(static_cast<hasher_type&>(other_)),
                KeyEqual(static_cast<key_equal&>(other_)),
                GrowthPolicy(static_cast<GrowthPolicy&>(other_)),
                _buckets(other_._buckets),
                _buckets_ptr(other_._buckets_ptr),
                _element_count(other_._element_count),
                _max_load_factor(other_._max_load_factor),
                _min_load_threshold(other_._min_load_threshold),
                _max_load_threshold(other_._max_load_threshold) {}

            /**
             * Move Constructor
             *
             * @author Julius
             * @date 28.10.2020
             *
             * @param [in,out] other_ The other.
             */
            hopscotch_hash(this_type&& other_) noexcept :
                Hash(_STD move(static_cast<hasher_type&>(other_))),
                KeyEqual(_STD move(static_cast<key_equal&>(other_))),
                GrowthPolicy(_STD move(static_cast<GrowthPolicy&>(other_))),
                _buckets(_STD move(other_._buckets)),
                _buckets_ptr(_buckets.empty() ? this_type::static_empty_bucket() : _buckets.data()),
                _element_count(_STD exchange(other_._element_count, 0)),
                _max_load_factor(_STD exchange(other_._max_load_factor, hopscotch::init_max_load_factor)),
                _min_load_threshold(_STD exchange(other_._min_load_threshold, 0)),
                _max_load_threshold(_STD exchange(other_._max_load_threshold, 0)) {
                /**
                 * Cleanup other_
                 */
                static_cast<GrowthPolicy&>(other_).clear();
                other_._buckets.clear();
                other_._buckets_ptr = this_type::static_empty_bucket();
            }

            /**
             * Copy Assignment operator
             *
             * @author Julius
             * @date 28.10.2020
             *
             * @param  other_ The other.
             *
             * @returns A shallow copy of this.
             */
            hopscotch_hash& operator=(const this_type& other_) = delete;

            /**
             * Move Assignment operator
             *
             * @author Julius
             * @date 28.10.2020
             *
             * @param [in,out] other_ The other.
             *
             * @returns A shallow copy of this.
             */
            hopscotch_hash& operator=(this_type&& other_) noexcept {
                static_cast<hasher_type&>(*this) = _STD move(static_cast<hasher_type&>(other_));
                static_cast<key_equal&>(*this) = _STD move(static_cast<key_equal&>(other_));
                static_cast<GrowthPolicy&>(*this) = _STD move(static_cast<GrowthPolicy&>(other_));

                _buckets = _STD move(other_._buckets);
                _buckets_ptr = _STD exchange(other_._buckets_ptr, nullptr);
                _element_count = _STD exchange(other_._element_count, 0);
                _max_load_factor = _STD exchange(other_._max_load_factor, 0);
                _min_load_threshold = _STD exchange(other_._min_load_threshold, 0);
                _max_load_threshold = _STD exchange(other_._max_load_threshold, 0);

                return *this;
            }

            /**
             * Gets the allocator
             *
             * @author Julius
             * @date 29.10.2020
             *
             * @returns An allocator_type.
             */
            allocator_type allocator() const {
                return _buckets.get_allocator();
            }

            /**
             * Hash function
             *
             * @author Julius
             * @date 28.10.2020
             *
             * @returns A reference to a const hasher_type.
             */
            const hasher_type& hash_function() const {
                return static_cast<const hasher_type&>(*this);
            }

            /**
             * Key equal function
             *
             * @author Julius
             * @date 28.10.2020
             *
             * @returns A reference to a const key_equal.
             */
            const key_equal& key_equal_function() const {
                return static_cast<const key_equal&>(*this);
            }

            /**
             * Check whether container is empty
             *
             * @author Julius
             * @date 28.10.2020
             *
             * @returns True if it succeeds, false if it fails.
             */
            [[nodiscard]] bool empty() const noexcept {
                return _element_count == 0;
            }

            /**
             * Gets the inserted element count of the container
             *
             * @author Julius
             * @date 28.10.2020
             *
             * @returns A size_type.
             */
            [[nodiscard]] size_type size() const noexcept {
                return _element_count;
            }

            /**
             * Gets the current capacity to hold elements in container
             *
             * @author Julius
             * @date 28.10.2020
             *
             * @returns A size_type.
             */
            [[nodiscard]] size_type capacity() const noexcept {
                return _buckets.capacity();
            }

            /**
             * Clears this to its blank/initial state
             *
             * @author Julius
             * @date 28.10.2020
             */
            void clear() noexcept {
                for (auto& entry : _buckets) {
                    entry.clear();
                }

                _element_count = 0;
            }

            /**
             * Emplaces the given value
             *
             * @author Julius
             * @date 28.10.2020
             *
             * @param  value_ The value.
             *
             * @returns A pair&lt;iterator,bool&gt;
             */
            [[nodiscard]] _STD pair<iterator, bool> emplace(const value_type& value_) {
                return insert_impl(value_);
            }

            /**
             * Emplaces the given value
             *
             * @tparam ValueType_ Type of the value type.
             * @param [in,out] value_ The value.
             *
             * @returns A pair&lt;iterator,bool&gt;
             */
            template <class ValueType_ = ValueType, _STD enable_if_t<_STD is_move_constructible_v<ValueType_>>>
            [[nodiscard]] _STD pair<iterator, bool> emplace(value_type&& value_) {
                return insert_impl(value_type { _STD forward<value_type>(value_) });
            }

            /**
             * Emplaces the given value
             *
             * @author Julius
             * @date 28.10.2020
             *
             * @param [in,out] value_ The value.
             *
             * @returns A pair&lt;iterator,bool&gt;
             */
            [[nodiscard]] _STD pair<iterator, bool> emplace(value_type&& value_) {
                return insert_impl(_STD move(value_));
            }

            /**
             * Try emplace
             *
             * @author Julius
             * @date 29.10.2020
             *
             * @param  value_ The value.
             *
             * @returns A pair&lt;iterator,bool&gt;
             */
            [[nodiscard]] _STD pair<iterator, bool> try_emplace(const value_type& value_) {
                return try_emplace_impl(value_);
            }

            /**
             * Try emplace
             *
             * @author Julius
             * @date 29.10.2020
             *
             * @param [in,out] value_ The value.
             *
             * @returns A pair&lt;iterator,bool&gt;
             */
            [[nodiscard]] _STD pair<iterator, bool> try_emplace(value_type&& value_) {
                return try_emplace_impl(_STD forward(value_));
            }

            /**
             * Erases the value at given position
             *
             * @author Julius
             * @date 28.10.2020
             *
             * @param  position_ The position.
             *
             * @returns An iterator.
             */
            iterator erase(iterator position_) {
                return erase(const_iterator(position_));
            }

            /**
             * Erases the value at given position
             *
             * @author Julius
             * @date 28.10.2020
             *
             * @param  position_ The position.
             *
             * @returns An iterator.
             */
            iterator erase(const_iterator position_) {
                const index_type idx = bucket_idx(hash(*position_));

                auto b = _buckets.begin() + _STD distance(_buckets.cbegin(), position_._buckets_iterator);
                bucket_erase(b, idx);

                return ++iterator(b, _buckets.end());
            }

            /**
             * Erases elements in range from_ to to_
             *
             * @author Julius
             * @date 28.10.2020
             *
             * @param  from_ Source for the.
             * @param  to_ to.
             *
             * @returns An iterator.
             */
            iterator erase(const_iterator from_, const_iterator to_) {
                if (from_ == to_) {
                    return mutable_iterator(from_);
                }

                auto d = erase(from_);
                while (d != to_) {
                    d = erase(d);
                }

                return d;
            }

            /**
             * Erases the given value from container
             *
             * @author Julius
             * @date 28.10.2020
             *
             * @param  value_ The value.
             *
             * @returns True if it succeeds, false if it fails.
             */
            bool erase(const value_type& value_) {
                return erase(value_, hash(value_));
            }

            /**
             * Erases the given value from container
             *
             * @author Julius
             * @date 28.10.2020
             *
             * @param  value_ The value.
             * @param  hash_ The hash.
             *
             * @returns True if it succeeds, false if it fails.
             */
            bool erase(const value_type& value_, hash_type hash_) {
                const index_type idx = bucket_idx(hash);

                bucket_type* bf = find_in_buckets(value_, hash_, _buckets_ptr + idx);
                if (bf != nullptr) {
                    bucket_erase(*bf, idx);
                    return true;
                }

                return false;
            }

            /**
             * Gets the const value reference at value position
             *
             * @author Julius
             * @date 28.10.2020
             *
             * @param  value_ The value.
             *
             * @returns A reference to a const value_type.
             */
            const value_type& at(const value_type& value_) const {
                return at(value_, hash(value_));
            }

            /**
             * Gets the const value reference at value position
             *
             * @author Julius
             * @date 28.10.2020
             *
             * @exception _STD Thrown when a Standard error condition occurs.
             *
             * @param  value_ The value.
             * @param  hash_ The hash.
             *
             * @returns A reference to a const value_type.
             */
            const value_type& at(const value_type& value_, hash_type hash_) const {
                const value_type* v = find_value_impl(value_, hash_, _buckets_ptr + bucket_idx(hash_));
                if (v == nullptr) {
                    throw _STD out_of_range("Couldn't find key.");
                }
                return *v;
            }

            /**
             * Get the value reference at value position
             *
             * @author Julius
             * @date 28.10.2020
             *
             * @param  value_ The value.
             *
             * @returns A reference to a value_type.
             */
            value_type& at(const value_type& value_) {
                return at(value_, hash(value_));
            }

            /**
             * Get the value reference at value position
             *
             * @author Julius
             * @date 28.10.2020
             *
             * @exception _STD Thrown when a Standard error condition occurs.
             *
             * @param  value_ The value.
             * @param  hash_ The hash.
             *
             * @returns A reference to a value_type.
             */
            value_type& at(const value_type& value_, hash_type hash_) {
                value_type* v = find_value_impl(value_, hash_, _buckets_ptr + bucket_idx(hash_));
                if (v == nullptr) {
                    throw _STD out_of_range("Couldn't find key.");
                }
                return *v;
            }

            /**
             * Get the value reference at value position
             *	Either the existing reference.
             *	Or insert a new element and return reference.
             *
             * @author Julius
             * @date 28.10.2020
             *
             * @param [in,out] value_ The value.
             *
             * @returns The indexed value.
             */
            value_type& operator[](value_type& value_) {
                const hash_type h = hash(value_);
                const index_type idx = bucket_idx(h);

                value_type* v = find_value_impl(value_, h, _buckets_ptr + idx);
                if (v != nullptr) {
                    return *v;
                }

                return insert_value(idx, h, value_);
            }

            /**
             * Counts the given value
             *
             * @author Julius
             * @date 28.10.2020
             *
             * @param  value_ The value.
             *
             * @returns A size_type.
             */
            size_type count(const value_type& value_) const {
                return count(value_, hash(value_));
            }

            /**
             * Counts the given value
             *
             * @author Julius
             * @date 28.10.2020
             *
             * @param  value_ The value.
             * @param  hash_ The hash.
             *
             * @returns A size_type.
             */
            size_type count(const value_type& value_, hash_type hash_) const {
                return count_impl(value_, hash_, _buckets_ptr + bucket_idx(hash_));
            }

            /**
             * Searches for the first match for the given constant value_
             *
             * @author Julius
             * @date 28.10.2020
             *
             * @param  value_ The value.
             *
             * @returns An iterator.
             */
            iterator find(const value_type& value_) {
                return find(value_, hash(value_));
            }

            /**
             * Searches for the first match for the given constant value_
             *
             * @author Julius
             * @date 28.10.2020
             *
             * @param  value_ The value.
             * @param  hash_ The hash.
             *
             * @returns An iterator.
             */
            iterator find(const value_type& value_, hash_type hash_) {
                return find_impl(value_, hash_, _buckets_ptr + bucket_idx(hash_));
            }

            /**
             * Searches for the first match for the given constant value_
             *
             * @author Julius
             * @date 28.10.2020
             *
             * @param  value_ The value.
             *
             * @returns A const_iterator.
             */
            const_iterator find(const value_type& value_) const {
                return find(value_, hash(value_));
            }

            /**
             * Searches for the first match for the given constant value_
             *
             * @author Julius
             * @date 28.10.2020
             *
             * @param  value_ The value.
             * @param  hash_ The hash.
             *
             * @returns A const_iterator.
             */
            const_iterator find(const value_type& value_, hash_type hash_) const {
                return find_impl(value_, hash_, _buckets_ptr + bucket_idx(hash_));
            }

            /**
             * Query if this contains the given value_
             *
             * @author Julius
             * @date 28.10.2020
             *
             * @param  value_ The const value_type&amp; to test for containment.
             *
             * @returns True if the object is in this collection, false if not.
             */
            bool contains(const value_type& value_) const {
                return contains(value_, hash(value_));
            }

            /**
             * Query if this contains the given value_
             *
             * @author Julius
             * @date 28.10.2020
             *
             * @param  value_ The value.
             * @param  hash_ The hash.
             *
             * @returns True if the object is in this collection, false if not.
             */
            bool contains(const value_type& value_, const hash_type hash_) const {
                return count(value_, hash_) != 0;
            }

            /**
             * Gets the bucket count
             *
             * @author Julius
             * @date 28.10.2020
             *
             * @returns A size_type.
             */
            [[nodiscard]] size_type bucket_count() const {
                return _buckets.empty() ? 0 : _buckets.size() - NeighborhoodSize + 1u;
            }

            /**
             * Gets the maximum bucket count
             *
             * @author Julius
             * @date 28.10.2020
             *
             * @returns A size_type.
             */
            [[nodiscard]] size_type max_bucket_count() const {
                const size_type mc = _STD min(GrowthPolicy::max_bucket_count(), _buckets.max_size());
                return mc - NeighborhoodSize + 1u;
            }

            /**
             * Loads the factor
             *
             * @author Julius
             * @date 28.10.2020
             *
             * @returns The factor.
             */
            [[nodiscard]] float load_factor() const {
                return bucket_count() /* Shorthand !(bucket_count() == 0) */ ?
                           (static_cast<float>(_element_count) / static_cast<float>(bucket_count())) :
                           0.F;
            }

            /**
             * Maximum load factor
             *
             * @author Julius
             * @date 28.10.2020
             *
             * @returns A float.
             */
            [[nodiscard]] float max_load_factor() const {
                return _max_load_factor;
            }

            /**
             * Rehashes this to reach the given count_
             *	Will automatically check and sanitize requested count_ to still hold every element
             *
             * @author Julius
             * @date 28.10.2020
             *
             * @param  count_ Number of.
             */
            void rehash(size_type count_) {
                const auto nc = _STD max(
                    count_,
                    static_cast<size_type>(
                        _STD ceil(
                            static_cast<float>(size()) / max_load_factor()
                        )
                    )
                );
                rehash_impl(nc);
            }

            /**
             * Reserves the given count
             *	Will automatically check and sanitize requested count_ to still hold every element
             *
             * @author Julius
             * @date 28.10.2020
             *
             * @param  count_ Number of.
             */
            void reserve(const size_type count_) {
                rehash(count_);
            }

            /**
             * Gets the begin
             *
             * @author Julius
             * @date 28.10.2020
             *
             * @returns An iterator.
             */
            [[nodiscard]] iterator begin() noexcept {
                auto bb = _buckets.begin();
                auto be = _buckets.end();
                while (bb != be && bb->empty())
                    ++bb;

                return iterator { bb, be };
            }

            /**
             * Gets the end
             *
             * @author Julius
             * @date 28.10.2020
             *
             * @returns An iterator.
             */
            [[nodiscard]] iterator end() noexcept {
                return iterator { _buckets.end(), _buckets.end() };
            }

            /**
             * Gets the begin
             *
             * @author Julius
             * @date 28.10.2020
             *
             * @returns A const_iterator.
             */
            [[nodiscard]] const_iterator begin() const noexcept {
                return cbegin();
            }

            /**
             * Gets the end
             *
             * @author Julius
             * @date 28.10.2020
             *
             * @returns A const_iterator.
             */
            [[nodiscard]] const_iterator end() const noexcept {
                return cend();
            }

            /**
             * Gets the cbegin
             *
             * @author Julius
             * @date 28.10.2020
             *
             * @returns A const_iterator.
             */
            [[nodiscard]] const_iterator cbegin() const noexcept {
                auto bb = _buckets.cbegin();
                auto be = _buckets.cend();
                while (bb != be && bb->empty())
                    ++bb;

                return const_iterator { bb, be };
            }

            /**
             * Gets the cend
             *
             * @author Julius
             * @date 28.10.2020
             *
             * @returns A const_iterator.
             */
            [[nodiscard]] const_iterator cend() const noexcept {
                return const_iterator { _buckets.cend(), _buckets.cend() };
            }

            /**
             * Swaps this with given other_
             *
             * @author Julius
             * @date 28.10.2020
             *
             * @param [in,out] other_ The other.
             */
            void swap(this_type& other_) noexcept {
                if (this == &other_)
                    return;

                _STD swap(static_cast<hasher_type&>(*this), static_cast<hasher_type&>(other_));
                _STD swap(static_cast<key_equal&>(*this), static_cast<key_equal&>(other_));
                _STD swap(static_cast<GrowthPolicy&>(*this), static_cast<GrowthPolicy&>(other_));

                swap(_buckets, other_._buckets);
                swap(_buckets_ptr, other_._buckets_ptr);
                swap(_element_count, other_._element_count);

                swap(_max_load_factor, other_._max_load_factor);
                swap(_min_load_threshold, other_._min_load_threshold);
                swap(_max_load_threshold, other_._max_load_threshold);
            }

        private:
            /**
             * Collection of buckets addressed by hopscotch hashing
             */
            bucket_container_type _buckets;
            bucket_type* _buckets_ptr;

            size_type _element_count;

            /**
             * Max load factor to rehash
             */
            float _max_load_factor;

            /**
             * The minimal load factor to trigger growth
             */
            size_type _min_load_threshold;

            /**
             * The maximum load factor to trigger growth
             */
            size_type _max_load_threshold;
        };
    }

    /**
     * A hopscotch set.
     *
     * @author Julius
     * @date 29.10.2020
     *
     * @tparam KeyType Type of the key type.
     * @tparam HashType Type of the hash type.
     * @tparam EqualType Type of the equal type.
     * @tparam CompareType Type of the compare type.
     * @tparam AllocatorType Type of the allocator type.
     * @tparam NeighborhoodSize Type of the neighborhood size.
     * @tparam StoreHash Type of the store hash.
     * @tparam GrowthPolicy Type of the growth policy.
     */
    template <typename KeyType,
        class HashType = std::hash<KeyType>,
        class EqualType = _STD equal_to<KeyType>,
        class CompareType = _STD less<KeyType>,
        class AllocatorType = _STD allocator<KeyType>,
        uint32_t NeighborhoodSize = 4,
        bool StoreHash = false,
        class GrowthPolicy = hopscotch::exp2_growth_policy<>>
    class hopscotch_set {
    private:
        using hopscotch_type = hopscotch::hopscotch_hash<KeyType, HashType, EqualType, AllocatorType, NeighborhoodSize,
            StoreHash, GrowthPolicy>;

    public:
        using key_type = KeyType;
        using hash_type = typename hopscotch_type::hash_type;

        using size_type = size_t;
        using difference_type = size_t;
        using key_equal = HashType;
        using key_compare = CompareType;

        using allocator_type = AllocatorType;

        using value_type = hopscotch_set<KeyType, HashType, EqualType, CompareType, AllocatorType, NeighborhoodSize,
            StoreHash, GrowthPolicy>;
        using reference_type = value_type&;
        using const_reference_type = const value_type&;

        using iterator = typename hopscotch_type::iterator;
        using const_iterator = typename hopscotch_type::const_iterator;

        using emplace_result_type = _STD pair<iterator, bool>;
        using insert_result_type = emplace_result_type;
        using insert_or_assign_result_type = emplace_result_type;

        /**
         * Default constructor
         *
         * @author Julius
         * @date 26.10.2020
         */
        hopscotch_set() :
            hopscotch_set(hopscotch::init_bucket_count) {}

        /**
         * Constructor
         *
         * @author Julius
         * @date 02.11.2020
         *
         * @param  reserve_ The reserve.
         * @param  hasher_ (Optional) The hasher.
         * @param  equal_ (Optional) The equal.
         * @param  compare_ (Optional) The compare.
         * @param  allocator_ (Optional) The allocator.
         */
        hopscotch_set(size_type reserve_, const HashType& hasher_ = HashType(), const EqualType& equal_ = EqualType(),
            const CompareType& compare_ = CompareType(), const AllocatorType& allocator_ = AllocatorType()) :
            _ht(reserve_, hasher_, equal_, allocator_, hopscotch::init_max_load_factor) {}

        /**
         * Copy constructor
         *
         * @author Julius
         * @date 26.10.2020
         *
         * @param  other_ The other.
         */
        hopscotch_set(const hopscotch_set& other_) :
            _ht(other_._ht) {}

        /**
         * Move constructor
         *
         * @author Julius
         * @date 26.10.2020
         *
         * @param [in,out] other_ The other.
         */
        hopscotch_set(hopscotch_set&& other_) noexcept :
            _ht(_STD move(other_._ht)) {}

        /**
         * Destructor
         *
         * @author Julius
         * @date 26.10.2020
         */
        ~hopscotch_set() = default;

        /**
         * Gets the begin
         *
         * @author Julius
         * @date 29.10.2020
         *
         * @returns An iterator.
         */
        [[nodiscard]] iterator begin() noexcept {
            return _ht.begin();
        }

        /**
         * Gets the end
         *
         * @author Julius
         * @date 29.10.2020
         *
         * @returns An iterator.
         */
        [[nodiscard]] iterator end() noexcept {
            return _ht.end();
        }

        /**
         * Gets the cbegin
         *
         * @author Julius
         * @date 29.10.2020
         *
         * @returns A const_iterator.
         */
        [[nodiscard]] const_iterator cbegin() const noexcept {
            return _ht.cbegin();
        }

        /**
         * Gets the cend
         *
         * @author Julius
         * @date 29.10.2020
         *
         * @returns A const_iterator.
         */
        [[nodiscard]] const_iterator cend() const noexcept {
            return _ht.cend();
        }

        /**
         * Empties this 
         *
         * @author Julius
         * @date 29.10.2020
         *
         * @returns True if it succeeds, false if it fails.
         */
        [[nodiscard]] bool empty() const noexcept {
            return _ht.empty();
        }

        /**
         * Gets the size
         *
         * @author Julius
         * @date 29.10.2020
         *
         * @returns A size_type.
         */
        [[nodiscard]] size_type size() const noexcept {
            return _ht.size();
        }

        /**
         * Gets the capacity
         *
         * @author Julius
         * @date 29.10.2020
         *
         * @returns A size_type.
         */
        [[nodiscard]] size_type capacity() const noexcept {
            return _ht.capacity();
        }

        /**
         * Clears this  to its blank/initial state
         *
         * @author Julius
         * @date 29.10.2020
         */
        void clear() noexcept {
            return _ht.clear();
        }

        /**
         * Emplaces the given value
         *
         * @author Julius
         * @date 29.10.2020
         *
         * @param  value_ The value.
         *
         * @returns A pair&lt;iterator,bool&gt;
         */
        emplace_result_type emplace(const key_type& value_) {
            return _ht.emplace(value_);
        }

        /**
         * Emplaces the given value
         *
         * @author Julius
         * @date 29.10.2020
         *
         * @param [in,out] value_ The value.
         *
         * @returns A pair&lt;iterator,bool&gt;
         */
        emplace_result_type emplace(key_type&& value_) {
            return _ht.emplace(_STD forward<key_type>(value_));
        }

        /**
         * Inserts an or assign the given value
         *
         * @author Julius
         * @date 30.10.2020
         *
         * @param  value_ The value.
         *
         * @returns An insert_or_assign_result_type.
         */
        insert_or_assign_result_type insert_or_assign(const key_type& value_) {
            return _ht.insert_or_assign(value_);
        }

        /**
         * Inserts an or assign the given value
         *
         * @author Julius
         * @date 30.10.2020
         *
         * @param [in,out] value_ The value.
         *
         * @returns An insert_or_assign_result_type.
         */
        insert_or_assign_result_type insert_or_assign(key_type&& value_) {
            return _ht.insert_or_assign(_STD forward<key_type>(value_));
        }

        /**
         * Erases the given position
         *
         * @author Julius
         * @date 29.10.2020
         *
         * @param  position_ The position.
         *
         * @returns An iterator.
         */
        iterator erase(iterator position_) {
            return _ht.erase(position_);
        }

        /**
         * Erases the given position
         *
         * @author Julius
         * @date 29.10.2020
         *
         * @param  position_ The position.
         *
         * @returns An iterator.
         */
        iterator erase(const_iterator position_) {
            return _ht.erase(position_);
        }

        /**
         * Counts the given value
         *
         * @author Julius
         * @date 29.10.2020
         *
         * @param  value_ The value.
         *
         * @returns A size_type.
         */
        [[nodiscard]] size_type count(const key_type& value_) const {
            return _ht.count(value_);
        }

        /**
         * Counts
         *
         * @author Julius
         * @date 29.10.2020
         *
         * @param  value_ The value.
         * @param  hash_ The hash.
         *
         * @returns A size_type.
         */
        [[nodiscard]] size_type count(const key_type& value_, const typename hopscotch_type::hash_type& hash_) const {
            return _ht.count(value_, hash_);
        }

        /**
         * Searches for the first match for the given constant value type&amp;
         *
         * @author Julius
         * @date 29.10.2020
         *
         * @param  value_ The value.
         *
         * @returns An iterator.
         */
        [[nodiscard]] iterator find(const key_type& value_) {
            return _ht.find(value_);
        }

        /**
         * Searches for the first match for the given constant value type&amp;
         *
         * @author Julius
         * @date 29.10.2020
         *
         * @param  value_ The value.
         * @param  hash_ The hash.
         *
         * @returns An iterator.
         */
        [[nodiscard]] iterator find(const key_type& value_, const hash_type& hash_) {
            return _ht.find(value_, hash_);
        }

        /**
         * Searches for the first match for the given constant value type&amp;
         *
         * @author Julius
         * @date 29.10.2020
         *
         * @param  value_ The value.
         *
         * @returns A const_iterator.
         */
        [[nodiscard]] const_iterator find(const key_type& value_) const {
            return _ht.find(value_);
        }

        /**
         * Searches for the first match for the given constant value type&amp;
         *
         * @author Julius
         * @date 29.10.2020
         *
         * @param  value_ The value.
         * @param  hash_ The hash.
         *
         * @returns A const_iterator.
         */
        [[nodiscard]] const_iterator find(const key_type& value_, const hash_type& hash_) const {
            return _ht.find(value_, hash_);
        }

        /**
         * Query if this  contains the given value_
         *
         * @author Julius
         * @date 29.10.2020
         *
         * @param  value_ The const value_type&amp; to test for containment.
         *
         * @returns True if the object is in this collection, false if not.
         */
        [[nodiscard]] bool contains(const key_type& value_) const {
            return _ht.contains(value_);
        }

        /**
         * Query if this  contains the given value_
         *
         * @author Julius
         * @date 29.10.2020
         *
         * @param  value_ The value.
         * @param  hash_ The hash.
         *
         * @returns True if the object is in this collection, false if not.
         */
        [[nodiscard]] bool contains(const key_type& value_, const typename hopscotch_type::hash_type& hash_) const {
            return _ht.contains(value_, hash_);
        }

        /**
         * Bucket count
         *
         * @author Julius
         * @date 29.10.2020
         *
         * @returns A size_type.
         */
        [[nodiscard]] size_type bucket_count() const {
            return _ht.bucket_count();
        }

        /**
         * Maximum bucket count
         *
         * @author Julius
         * @date 29.10.2020
         *
         * @returns A size_type.
         */
        [[nodiscard]] size_type max_bucket_count() const {
            return _ht.max_bucket_count();
        }

        /**
         * Loads the factor
         *
         * @author Julius
         * @date 29.10.2020
         *
         * @returns The factor.
         */
        [[nodiscard]] float load_factor() const {
            return _ht.load_factor();
        }

        /**
         * Maximum load factor
         *
         * @author Julius
         * @date 29.10.2020
         *
         * @returns A float.
         */
        [[nodiscard]] float max_load_factor() const {
            return _ht.max_load_factor();
        }

        /**
         * Rehashes the given count
         *
         * @author Julius
         * @date 29.10.2020
         *
         * @param  count_ Number of.
         */
        void rehash(size_type count_) {
            _ht.rehash(count_);
        }

        /**
         * Reserves the given count
         *
         * @author Julius
         * @date 29.10.2020
         *
         * @param  count_ Number of.
         */
        void reserve(size_type count_) {
            _ht.reserve(count_);
        }

        /**
         * Hash function
         *
         * @author Julius
         * @date 29.10.2020
         *
         * @returns A HashType.
         */
        [[nodiscard]] HashType hash_function() const {
            return _ht.hash_function();
        }

        /**
         * Key equal function
         *
         * @author Julius
         * @date 29.10.2020
         *
         * @returns A key_equal.
         */
        [[nodiscard]] key_equal key_equal_function() const {
            return _ht.key_equal_function();
        }

        /**
         * Key component function
         *
         * @author Julius
         * @date 29.10.2020
         *
         * @returns A key_compare.
         */
        [[nodiscard]] key_compare key_comp_function() const {
            return _ht.key_compare_function();
        }

    private:
        hopscotch_type _ht;
    };
}
