#pragma once

#include <algorithm>
#include <stdexcept>

#include "Bucket.hpp"
#include "GrowthPolicy.hpp"

#include "../../Types.hpp"
#include "../List.hpp"

namespace hg::hopscotch {
    inline constexpr static size_t init_bucket_count = 0;

    /**
     * The initial minimum load threshold factor
     */
    inline constexpr static float init_min_load_threshold_factor = 0.1f;

    /**
     * The initial maximum load factor
     */
    inline constexpr static float init_max_load_factor = 0.8f;

    template <typename KeyType_, typename IteratorType_, typename ConstIteratorType_, typename ValueType_, bool Mapped_
        = false>
    struct select_key_t {
        [[nodiscard]] FORCE_INLINE cref<KeyType_> operator()(cref<IteratorType_> iterator_) const noexcept {
            return *iterator_;
        }

        [[nodiscard]] FORCE_INLINE cref<KeyType_> operator()(cref<ConstIteratorType_> iterator_) const noexcept {
            return *iterator_;
        }

        [[nodiscard]] FORCE_INLINE cref<KeyType_> operator()(cref<ValueType_> value_) const noexcept {
            return value_;
        }
    };

    template <typename KeyType_, typename IteratorType_, typename ConstIteratorType_, typename ValueType_>
    struct select_key_t<KeyType_, IteratorType_, ConstIteratorType_, ValueType_, true> {
        [[nodiscard]] FORCE_INLINE cref<KeyType_> operator()(cref<IteratorType_> iterator_) const noexcept {
            return iterator_.key();
        }

        [[nodiscard]] FORCE_INLINE cref<KeyType_> operator()(cref<ConstIteratorType_> iterator_) const noexcept {
            return iterator_.key();
        }

        [[nodiscard]] FORCE_INLINE cref<KeyType_> operator()(cref<ValueType_> value_) const noexcept {
            return value_.first;
        }
    };

    template <
        class KeyType_,
        class MappedType_,
        class HasherType_,
        class KeyEqualsType_,
        class Allocator,
        u8 NeighborhoodSize_,
        bool StoreHash_,
        IsGrowthPolicy GrowthPolicyType_,
        class OverflowType_
    >
    class hopscotch_hash :
        private HasherType_,
        private KeyEqualsType_,
        private GrowthPolicyType_ {
    public:
        using this_type = hopscotch_hash<KeyType_, MappedType_, HasherType_, KeyEqualsType_,
            Allocator, NeighborhoodSize_, StoreHash_, GrowthPolicyType_, OverflowType_>;

        using hash_type = u64;
        using index_type = _STD size_t;
        using size_type = _STD size_t;

        using difference_type = _STD ptrdiff_t;
        using hasher_type = HasherType_;
        using key_equal_type = KeyEqualsType_;

        constexpr static bool is_mapped_container = !_STD is_void_v<MappedType_>;

        using key_type = KeyType_;
        using mapped_type = MappedType_;

        using value_type = _STD conditional_t<is_mapped_container, _STD pair<key_type, mapped_type>, key_type>;

        using allocator_type = Allocator;
        using allocator_traits = _STD allocator_traits<allocator_type>;

        using bucket_type = hopscotch_bucket<value_type, NeighborhoodSize_, StoreHash_>;
        using bucket_allocator = typename allocator_traits::template rebind_alloc<bucket_type>;
        using bucket_container_type = vector<bucket_type>;

        using bucket_iterator = typename bucket_container_type::iterator;
        using bucket_const_iterator = typename bucket_container_type::const_iterator;

        using overflow_container = OverflowType_;

        using overflow_iterator = typename overflow_container::iterator;
        using overflow_const_iterator = typename overflow_container::const_iterator;

    public:
        template <bool Const_>
        class hopscotch_iterator final {
        private:
            friend class hopscotch_hash;

        protected:
            using bucket_iterator = _STD conditional_t<Const_,
                typename hopscotch_hash::bucket_const_iterator,
                typename hopscotch_hash::bucket_iterator
            >;

            using overflow_iterator = _STD conditional_t<Const_,
                typename hopscotch_hash::overflow_const_iterator,
                typename hopscotch_hash::overflow_iterator
            >;

        public:
            using iterator_category = _STD forward_iterator_tag;

            using value_type = _STD conditional_t<Const_,
                const typename hopscotch_hash::value_type,
                typename hopscotch_hash::value_type
            >;

            using difference_type = _STD ptrdiff_t;
            using reference = ref<value_type>;
            using pointer = ptr<value_type>;

            using this_type = hopscotch_iterator<Const_>;
            using reference_this_type = ref<this_type>;
            using const_reference_this_type = cref<this_type>;

        public:
            /**
             * Default constructor
             *
             * @author Julius
             * @date 04.10.2021
             */
            hopscotch_iterator() noexcept = default;

        protected:
            /**
             * Constructor
             *
             * @author Julius
             * @date 04.10.2021
             *
             * @param  cur_ The current iterator.
             * @param  end_ The end iterator.
             * @param   overflow_iterator_ The overflow iterator.
             */
            hopscotch_iterator(
                bucket_iterator cur_,
                bucket_iterator end_,
                overflow_iterator overflow_iterator_
            ) noexcept :
                _bucketIterator(cur_),
                _bucketEnd(end_),
                _overflowIterator(overflow_iterator_) {}

        public:
            /**
             * Copy Constructor
             *
             * @author Julius
             * @date 04.10.2021
             *
             * @param  other_ The other.
             */
            hopscotch_iterator(const_reference_this_type other_) = default;

            /**
             * Move Constructor
             *
             * @author Julius
             * @date 04.10.2021
             *
             * @param  other_ The other.
             */
            hopscotch_iterator(mref<this_type> other_) noexcept = default;

            /**
             * Cast Constructor [Const => NonConst]
             *
             * @author Julius
             * @date 04.10.2021
             *
             * @tparam Const_ Type of the constant.
             * @param  other_ The other.
             */
            template <bool Const_ = Const_, typename = _STD enable_if_t<Const_>>
            hopscotch_iterator(typename hopscotch_iterator<!Const_>::const_reference_this_type other_) noexcept :
                _bucketIterator(other_._bucketIterator),
                _bucketEnd(other_._bucketEnd),
                _overflowIterator(other_._overflowIterator) {}

        public:
            /**
             * Move Assignment operator
             *
             * @author Julius
             * @date 04.10.2021
             *
             * @param  other_ The other.
             *
             * @returns A shallow copy of this.
             */
            reference_this_type operator=(const_reference_this_type other_) {

                if (this != _STD addressof(other_)) {
                    _bucketIterator = other_._bucketIterator;
                    _bucketEnd = other_._bucketEnd;
                    _overflowIterator = other_._overflowIterator;
                }

                return *this;
            }

            /**
             * Move Assignment operator
             * 
             * @author Julius
             * @date 04.10.2021
             *
             * @param  other_ The other.
             *
             * @returns A shallow copy of this.
             */
            reference_this_type operator=(mref<this_type> other_) noexcept {

                if (this != _STD addressof(other_)) {
                    _bucketIterator = _STD move(other_._bucketIterator);
                    _bucketEnd = _STD move(other_._bucketEnd);
                    _overflowIterator = _STD move(other_._overflowIterator);
                }

                return *this;
            }

        private:
            bucket_iterator _bucketIterator;
            bucket_iterator _bucketEnd;
            overflow_iterator _overflowIterator;

        public:
            /**
             * Check whether this iterator is in overflow range
             *
             * @returns True if it succeeds, false if it fails.
             */
            [[nodiscard]] bool overflow() const noexcept {
                return _bucketIterator == _bucketEnd;
            }

            #if FALSE
        public:
            /**
             * Gets the current key
             *
             * @returns A cref&lt;typename hopscotch_hash::key_type&gt;
             */
            template <typename = _STD enable_if_t<is_mapped_container>>
            cref<key_type> key() const {

                if (_bucketIterator != _bucketEnd) {
                    return _bucketIterator->value().first;
                }

                return (*_overflowIterator).first;
            }

            /**
             * Gets the current value
             *
             * @returns A reference.
             */
            template <typename = _STD enable_if_t<!_STD is_void_v<MappedType_>>>
            [[nodiscard]] _STD conditional_t<Const_, cref<mapped_type>, cref<mapped_type>> value() const {

                if (_bucketIterator != _bucketEnd) {
                    return _bucketIterator->value().second;
                }

                return (*_overflowIterator).second;
            }
            #endif

        public:
            /**
             * Indirection operator
             *
             * @author Julius
             * @date 04.10.2021
             *
             * @returns A reference to the iterator value
             */
            [[nodiscard]] cref<typename hopscotch_hash::value_type> operator*() const {
                if (_bucketIterator != _bucketEnd) {
                    return _bucketIterator->value();
                }

                return *_overflowIterator;
            }

            /**
             * Indirection operator
             *
             * @author Julius
             * @date 04.10.2021
             *
             * @returns A reference to the iterator value
             */
            [[nodiscard]] ref<typename hopscotch_hash::value_type> operator*() {
                if (_bucketIterator != _bucketEnd) {
                    return const_cast<ref<typename hopscotch_hash::value_type>>(_bucketIterator->value());
                }

                return const_cast<ref<typename hopscotch_hash::value_type>>(*_overflowIterator);
            }

            /**
             * Member dereference operator
             *
             * @author Julius
             * @date 04.10.2021
             *
             * @returns A pointer to the iterator value
             */
            [[nodiscard]] ptr<const typename hopscotch_hash::value_type> operator->() const {
                if (_bucketIterator != _bucketEnd) {
                    return _STD addressof(_bucketIterator->value());
                }

                return _STD addressof(*_overflowIterator);
            }

            /**
             * Member dereference operator
             *
             * @author Julius
             * @date 04.10.2021
             *
             * @returns A pointer to the iterator value
             */
            [[nodiscard]] ptr<typename hopscotch_hash::value_type> operator->() {
                if (_bucketIterator != _bucketEnd) {
                    return const_cast<ptr<typename hopscotch_hash::value_type>>(
                        _STD addressof(_bucketIterator->value()));
                }

                return const_cast<ptr<typename hopscotch_hash::value_type>>(_STD addressof(*_overflowIterator));
            }

        public:
            /**
             * Pre Increment operator
             *
             * @author Julius
             * @date 04.10.2021
             *
             * @returns The result of the operation.
             */
            reference_this_type operator++() {
                if (_bucketIterator == _bucketEnd) {
                    ++_overflowIterator;
                    return *this;
                }

                do {
                    ++_bucketIterator;
                } while (_bucketIterator != _bucketEnd && _bucketIterator->empty());

                return *this;
            }

            /**
             * Post Increment operator
             *
             * @author Julius
             * @date 04.10.2021
             *
             * @returns The result of the operation.
             */
            this_type operator++(int) {
                auto tmp { *this };
                ++*this;
                return tmp;
            }

        public:
            /**
             * Equality operator
             *
             * @author Julius
             * @date 04.10.2021
             *
             * @param  left_ The first instance to compare.
             * @param  right_ The second instance to compare.
             *
             * @returns True if the parameters are considered equivalent.
             */
            friend bool operator==(const_reference_this_type left_, const_reference_this_type right_) {
                return left_._bucketIterator == right_._bucketIterator && left_._overflowIterator == right_.
                    _overflowIterator;
            }

            /**
             * Inequality operator
             *
             * @author Julius
             * @date 04.10.2021
             *
             * @param  left_ The first instance to compare.
             * @param  right_ The second instance to compare.
             *
             * @returns True if the parameters are not considered equivalent.
             */
            friend bool operator!=(const_reference_this_type left_, const_reference_this_type right_) {
                return left_._bucketIterator != right_._bucketIterator || left_._overflowIterator != right_.
                    _overflowIterator;
            }
        };

        using iterator = hopscotch_iterator<false>;
        using const_iterator = hopscotch_iterator<true>;

    public:
        /**
         * Constructor
         *
         * @author Julius
         * @date 04.10.2021
         *
         * @param  buckets_ The buckets.
         * @param  hasher_ The hasher.
         * @param  keyEquals_ The key equals.
         * @param  allocator_ The allocator.
         * @param  maxLoadFactor_ The maximum load factor.
         */
        hopscotch_hash(
            size_type buckets_,
            cref<hasher_type> hasher_,
            cref<key_equal_type> keyEquals_,
            cref<allocator_type> allocator_,
            float maxLoadFactor_
        ) :
            HasherType_(hasher_),
            KeyEqualsType_(keyEquals_),
            GrowthPolicyType_(buckets_),
            _buckets(allocator_),
            _buckets_ptr(this_type::static_empty_bucket()),
            _overflow(allocator_),
            _max_load_factor(maxLoadFactor_),
            _element_count(0) {

            if (buckets_ > 0) {
                _buckets.resize(buckets_ + NeighborhoodSize_ - 1u);
                _buckets_ptr = _buckets.data();
            }

            update_load_meta(maxLoadFactor_);
        }

        /**
         * Copy Constructor
         *
         * @author Julius
         * @date 04.10.2021
         *
         * @param  other_ The other.
         */
        hopscotch_hash(cref<this_type> other_) :
            HasherType_(other_),
            KeyEqualsType_(other_),
            GrowthPolicyType_(other_),
            _buckets(other_._buckets),
            _buckets_ptr(other_._buckets_ptr),
            _overflow(other_._overflow),
            _max_load_factor(other_._max_load_factor),
            _element_count(other_._element_count),
            _min_load_threshold(other_._min_load_threshold),
            _max_load_threshold(other_._max_load_threshold) {}

        /**
         * Move Constructor
         *
         * @author Julius
         * @date 04.10.2021
         *
         * @param  other_ The other.
         */
        hopscotch_hash(mref<this_type> other_) noexcept :
            HasherType_(_STD move(static_cast<ref<HasherType_>>(other_))),
            KeyEqualsType_(_STD move(static_cast<ref<KeyEqualsType_>>(other_))),
            GrowthPolicyType_(_STD move(static_cast<ref<GrowthPolicyType_>>(other_))),
            _buckets(_STD move(other_._buckets)),
            _buckets_ptr(_buckets.empty() ? this_type::static_empty_bucket() : _buckets.data()),
            _overflow(_STD move(other_._overflow)),
            _max_load_factor(_STD exchange(other_._max_load_factor, init_max_load_factor)),
            _element_count(_STD exchange(other_._element_count, 0)),
            _min_load_threshold(_STD exchange(other_._min_load_threshold, 0)),
            _max_load_threshold(_STD exchange(other_._max_load_threshold, 0)) {

            other_.GrowthPolicyType_::clear();
            other_._buckets.clear();
            other_._buckets_ptr = this_type::static_empty_bucket();
            other_._overflow.clear();
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
        ref<this_type> operator=(cref<this_type> other_) {

            if (this != _STD addressof(other_)) {
                HasherType_::operator=(other_);
                KeyEqualsType_::operator=(other_);
                GrowthPolicyType_::operator=(other_);

                _buckets = other_._buckets;
                _buckets_ptr = _buckets.empty() ? this_type::static_empty_bucket() : _buckets.data();
                _overflow = other_._overflow;

                _max_load_threshold = other_._max_load_threshold;
                _element_count = other_._element_count;
                _min_load_threshold = other_._min_load_threshold;
                _max_load_threshold = other_._max_load_threshold;
            }

            return *this;
        }

        /**
         * Move Assignment operator
         *
         * @author Julius
         * @date 04.10.2021
         *
         * @param  other_ The other.
         *
         * @returns A shallow copy of this.
         */
        ref<this_type> operator=(mref<this_type> other_) noexcept {

            if (this != _STD addressof(other_)) {
                other_.swap(*this);
                other_.clear();
            }

            return *this;
        }

    public:
        void swap(ref<this_type> other_) {
            using _STD swap;

            swap(static_cast<ref<HasherType_>>(*this), static_cast<ref<HasherType_>>(other_));
            swap(static_cast<ref<KeyEqualsType_>>(*this), static_cast<ref<KeyEqualsType_>>(other_));
            swap(static_cast<ref<GrowthPolicyType_>>(*this), static_cast<ref<GrowthPolicyType_>>(other_));

            swap(_buckets, other_._buckets);
            swap(_buckets_ptr, other_._buckets_ptr);
            swap(_overflow, other_._overflow);

            swap(_element_count, other_._element_count);
            swap(_max_load_factor, other_._max_load_factor);
            swap(_min_load_threshold, other_._min_load_threshold);
            swap(_max_load_threshold, other_._max_load_threshold);
        }

    private:
        bucket_container_type _buckets;
        ptr<bucket_type> _buckets_ptr;

        overflow_container _overflow;

        float _max_load_factor;

        size_type _element_count;
        size_type _min_load_threshold;
        size_type _max_load_threshold;

    public:
        /**
         * Gets the allocator
         *
         * @author Julius
         * @date 04.10.2021
         *
         * @returns An allocator_type.
         */
        [[nodiscard]] allocator_type allocator() const {
            return _buckets.get_allocator();
        }

        /**
         * Gets the hasher operator
         *
         * @author Julius
         * @date 04.10.2021
         *
         * @returns A HasherType_.
         */
        [[nodiscard]] HasherType_ hasher() const {
            return static_cast<cref<HasherType_>>(*this);
        }

        /**
         * Gets the key equals operator
         *
         * @author Julius
         * @date 04.10.2021
         *
         * @returns A KeyEqualsType_.
         */
        [[nodiscard]] KeyEqualsType_ key_equals() const {
            return static_cast<cref<KeyEqualsType_>>(*this);
        }

    public:
        /**
         * Check whether this is empty
         *
         * @author Julius
         * @date 04.10.2021
         *
         * @returns True if it succeeds, false if it fails.
         */
        [[nodiscard]] bool empty() const noexcept {
            return _element_count == 0;
        }

        /**
         * Gets the amount of elements
         *
         * @author Julius
         * @date 04.10.2021
         *
         * @returns A size_type.
         */
        [[nodiscard]] size_type size() const noexcept {
            return _element_count;
        }

        /**
         * Gets the amount of elements in overflow
         *
         * @author Julius
         * @date 04.10.2021
         *
         * @returns A size_type.
         */
        [[nodiscard]] size_type overflow_size() const noexcept {
            return _overflow.size();
        }

        /**
         * Maximum amount of storeable elements
         *
         * @author Julius
         * @date 04.10.2021
         *
         * @returns The maximum size of the.
         */
        [[nodiscard]] size_type max_size() const noexcept {
            return _buckets.max_size();
        }

        /**
         * Gets the current capacity to hold elements
         *
         * @author Julius
         * @date 04.10.2021
         *
         * @returns A size_type.
         */
        [[nodiscard]] size_type capacity() const noexcept {
            return _buckets.capacity();
        }

    public:
        /**
         * Bucket count
         *
         * @author Julius
         * @date 04.10.2021
         *
         * @returns A size_type.
         */
        [[nodiscard]] size_type bucket_count() const noexcept {
            return _buckets.empty() ? 0 : _buckets.size() - NeighborhoodSize_ + 1u;
        }

        /**
         * Maximum bucket count
         *
         * @author Julius
         * @date 04.10.2021
         *
         * @returns A size_type.
         */
        [[nodiscard]] size_type max_bucket_count() const noexcept {
            return MIN(GrowthPolicyType_::max_bucket_count(), _buckets.max_size()) - NeighborhoodSize_ + 1u;
        }

        /**
         * Get the current load factor
         *
         * @author Julius
         * @date 04.10.2021
         *
         * @returns The factor.
         */
        [[nodiscard]] float load_factor() const noexcept {
            const auto bc { bucket_count() };
            return bc ? static_cast<float>(_element_count) / static_cast<float>(bc) : 0.F;
        }

        /**
         * Maximum load factor
         *
         * @author Julius
         * @date 04.10.2021
         *
         * @returns A float.
         */
        [[nodiscard]] float max_load_factor() const noexcept {
            return _max_load_factor;
        }

    public:
        /**
         * Clears this to its blank/initial state
         *
         * @author Julius
         * @date 04.10.2021
         */
        void clear() noexcept {
            for (auto& entry : _buckets) {
                entry.clear();
            }

            _overflow.clear();
            _element_count = 0;
        }

        /**
         * Rehashes this to hold given count of elements
         *
         * @author Julius
         * @date 04.10.2021
         *
         * @param  count_ Number of.
         */
        void rehash(size_type count_) {
            count_ = _STD max<size_type>(
                count_,
                static_cast<size_type>(_STD ceil(static_cast<float>(size()) / max_load_factor()))
            );
            rehash_impl(count_);
        }

        /**
         * Reserves the given count
         *
         * @author Julius
         * @date 04.10.2021
         *
         * @param  count_ Number of.
         */
        void reserve(size_type count_) {
            rehash(static_cast<size_type>(_STD ceil(static_cast<float>(count_) / max_load_factor())));
        }

    public:
        /**
         * Gets the iterator of the begin
         *
         * @author Julius
         * @date 04.10.2021
         *
         * @returns An iterator.
         */
        [[nodiscard]] iterator begin() noexcept {
            auto bb = _buckets.begin();
            auto be = _buckets.end();

            while (bb != be && bb->empty()) {
                ++bb;
            }

            return iterator { bb, be, _overflow.begin() };
        }

        /**
         * Gets the iterator of the begin
         *
         * @author Julius
         * @date 04.10.2021
         *
         * @returns A const_iterator.
         */
        [[nodiscard]] const_iterator cbegin() const noexcept {
            return cbegin();
        }

        /**
         * Gets the const_iterator of the begin
         *
         * @author Julius
         * @date 04.10.2021
         *
         * @returns A const_iterator.
         */
        [[nodiscard]] const_iterator cbegin() noexcept {
            auto bb = _buckets.begin();
            auto be = _buckets.end();

            while (bb != be && bb->empty()) {
                ++bb;
            }

            return const_iterator { bb, be, _overflow.begin() };
        }

        /**
         * Gets the iterator of the end
         *
         * @author Julius
         * @date 04.10.2021
         *
         * @returns An iterator.
         */
        [[nodiscard]] iterator end() noexcept {
            return iterator { _buckets.end(), _buckets.end(), _overflow.end() };
        }

        /**
         * Gets the iterator of the end
         *
         * @author Julius
         * @date 04.10.2021
         *
         * @returns A const_iterator.
         */
        [[nodiscard]] const_iterator end() const noexcept {
            return cend();
        }

        /**
         * Gets the const_iterator of the end
         *
         * @author Julius
         * @date 04.10.2021
         *
         * @returns A const_iterator.
         */
        [[nodiscard]] const_iterator cend() const noexcept {
            return const_iterator { _buckets.end(), _buckets.end(), _overflow.end() };
        }

    public:
        _STD pair<iterator, bool> insert(cref<value_type> value_) {
            return insert_impl(value_);
        }

        _STD pair<iterator, bool> insert(mref<value_type> value_) {
            return insert_impl(_STD move(value_));
        }

    public:
        _STD pair<iterator, bool> insert_or_assign(cref<value_type> value_) {
            return insert_or_assign_impl(value_);
        }

        _STD pair<iterator, bool> insert_or_assign(mref<value_type> value_) {
            return insert_or_assign_impl(_STD move(value_));
        }

    public:
        template <class... Args_>
        _STD pair<iterator, bool> emplace(Args_&&... args_) {
            return insert(value_type(_STD forward<Args_>(args_)...));
        }

        _STD pair<iterator, bool> try_emplace(mref<value_type> value_) {
            return try_emplace_impl(_STD move(value_));
        }

        template <class... Args_>
        _STD pair<iterator, bool> try_emplace(cref<key_type> key_, Args_&&... args_) {
            return try_emplace_impl(key_, _STD forward<Args_>(args_)...);
        }

    public:
        iterator erase(iterator where_) {
            return erase(const_iterator(where_));
        }

        iterator erase(const_iterator where_) {

            const index_type idx = hash_to_bucket(hash_key(select_key(where_)));

            if (where_._bucketIterator != where_._bucketEnd) {

                auto it = _buckets.begin() + _STD distance(_buckets.cbegin(), where_._bucketIterator);
                erase_from_bucket(*it, idx);

                return ++iterator { it, _buckets.end(), _overflow.begin() };

            } else {

                auto it = erase_from_overflow(where_._overflowIterator, idx);

                return iterator { _buckets.end(), _buckets.end(), it };
            }
        }

        iterator erase(const_iterator first_, const_iterator last_) {
            if (first_ == last_) {
                return mutable_iterator(first_);
            }

            auto ff = erase(first_);
            while (ff != last_) {
                ff = erase(ff);
            }

            return ff;
        }

        bool erase(cref<key_type> key_) {
            return erase(key_, hash_key(key_));
        }

        bool erase(cref<key_type> key_, const hash_type hash_) {

            const index_type idx = hash_to_bucket(hash_);
            auto* found = find_in_buckets(key_, hash_, _buckets_ptr + idx);

            if (found != nullptr) {
                erase_from_bucket(*found, idx);
                return true;
            }

            if (_buckets_ptr[idx].overflow()) {
                auto it = find_in_overflow(key_);

                if (it != _overflow.end()) {
                    erase_from_overflow(it, idx);
                    return true;
                }
            }

            return false;
        }

    public:
        ref<value_type> at(cref<key_type> key_) {
            return at(key_, hash_key(key_));
        }

        ref<value_type> at(cref<key_type> key_, hash_type hash_) {

            const ptr<value_type> pv = find_value_impl(
                key_,
                hash_,
                _buckets_ptr + hash_to_bucket(hash_)
            );

            if (pv == nullptr) {
                throw _STD out_of_range("Couldn't find key.");
            }

            return *pv;
        }

        cref<value_type> at(cref<key_type> key_) const {
            return at(key_, hash_key(key_));
        }

        cref<value_type> at(cref<key_type> key_, hash_type hash_) const {

            const ptr<value_type> pv = find_value_impl(
                key_,
                hash_,
                _buckets_ptr + hash_to_bucket(hash_)
            );

            if (pv == nullptr) {
                throw _STD out_of_range("Couldn't find key.");
            }

            return *pv;
        }

    public:
        ref<value_type> operator[](cref<key_type> key_) {

            const hash_type hk = hash_key(key_);
            const index_type idx = hash_to_bucket(hk);

            ptr<value_type> pv = find_value_impl(key_, hk_, _buckets_ptr + idx);

            if (pv != nullptr) {
                return *pv;

            }

            return insert_value(
                idx,
                hk,
                _STD piecewise_construct,
                _STD forward_as_tuple(key_),
                _STD forward_as_tuple()
            ).first.value();
        }

    public:
        [[nodiscard]] size_type count(cref<key_type> key_) const {
            return count(key_, hash_key(key_));
        }

        [[nodiscard]] size_type count(cref<key_type> key_, hash_type hash_) const {
            return count_impl(key_, hash_, _buckets_ptr + hash_to_bucket(hash_));
        }

    public:
        [[nodiscard]] iterator find(cref<key_type> key_) {
            return find(key_, hash_key(key_));
        }

        [[nodiscard]] iterator find(cref<key_type> key_, hash_type hash_) {
            return find_impl(key_, hash_, _buckets_ptr + hash_to_bucket(hash_));
        }

        [[nodiscard]] const_iterator find(cref<key_type> key_) const {
            return find(key_, hash_key(key_));
        }

        [[nodiscard]] const_iterator find(cref<key_type> key_, hash_type hash_) const {
            return const_cast<this_type*>(this)->find_impl(key_, hash_, _buckets_ptr + hash_to_bucket(hash_));
        }

    public:
        [[nodiscard]] bool contains(cref<key_type> key_) const {
            return contains(key_, hash_key(key_));
        }

        [[nodiscard]] bool contains(cref<key_type> key_, hash_type hash_) const {
            return count(key_, hash_) != 0;
        }

    public:
        iterator mutable_iterator(const_iterator where_) {
            if (where_._bucketIterator != where_._bucketEnd) {

                auto it = _buckets.begin() + _STD distance(_buckets.cbegin(), where_._bucketIterator);
                return iterator { it, _buckets.end(), _overflow.begin() };

            } else {

                auto it = mutable_overflow_iterator(where_._overflowIterator);
                return iterator { _buckets.end(), _buckets.end(), it };

            }
        }

        overflow_iterator mutable_overflow_iterator(overflow_const_iterator where_) {
            return _overflow.begin() + _STD distance(static_cast<cref<this_type>>(*this)._overflow.begin(), where_);
        }

    private:
        [[nodiscard]] FORCE_INLINE static cref<key_type> select_key(cref<iterator> data_) {
            return select_key_t<KeyType_, iterator, const_iterator, value_type, is_mapped_container> {}(data_);
        }

        [[nodiscard]] FORCE_INLINE static cref<key_type> select_key(cref<const_iterator> data_) {
            return select_key_t<KeyType_, iterator, const_iterator, value_type, is_mapped_container> {}(data_);
        }

        [[nodiscard]] FORCE_INLINE static cref<key_type> select_key(cref<value_type> data_) {
            return select_key_t<KeyType_, iterator, const_iterator, value_type, is_mapped_container> {}(data_);
        }

        /**
         * Hashes the given key
         *
         * @author Julius
         * @date 04.10.2021
         *
         * @param  key_ The key.
         *
         * @returns A hash_type.
         */
        [[nodiscard]] hash_type hash_key(cref<key_type> key_) const {
            return HasherType_::operator()(key_);
        }

        /**
         * Compares two key objects to determine their relative ordering
         *
         * @author Julius
         * @date 04.10.2021
         *
         * @param  left_ The left.
         * @param  right_ The right.
         *
         * @returns True if it succeeds, false if it fails.
         */
        [[nodiscard]] bool cmp(cref<key_type> left_, cref<key_type> right_) const noexcept {
            return KeyEqualsType_::operator()(left_, right_);
        }

        /**
         * Resolves the bucket index by given hash with GrowthPolicy
         *
         * @author Julius
         * @date 04.10.2021
         *
         * @param  hash_ The hash.
         *
         * @returns An index_type.
         */
        [[nodiscard]] index_type hash_to_bucket(cref<hash_type> hash_) const noexcept {
            const index_type idx = hash_to_bucket(hash_);
            DEBUG_ASSERT(
                idx < _buckets.size() || (idx == 0 && _buckets.empty()),
                "Value of 'idx' by hash should be a valid bucket."
            )
            return idx;
        }

        /**
         * Use stored hash
         *
         * @author Julius
         * @date 04.10.2021
         *
         * @param  count_ Number of.
         *
         * @returns True if it succeeds, false if it fails.
         */
        [[nodiscard]] static bool use_stored_hash(const size_type count_) noexcept {
            return count_ < limits<hash_truncate_type>::max;
        }

        /**
         * Updates the load stats effected by factor_
         *
         * @author Julius
         * @date 04.10.2021
         *
         * @param  factor_ The factor.
         */
        void update_load_meta(const float factor_) noexcept {
            _max_load_factor = MAX(0.1F, MIN(factor_, 0.95F));
            _min_load_threshold = static_cast<size_type>(static_cast<float>(bucket_count()) *
                init_min_load_threshold_factor);
            _max_load_threshold = static_cast<size_type>(static_cast<float>(bucket_count()) * _max_load_factor);
        }

    private:
        FORCE_INLINE this_type deduce(size_type buckets_) const {
            return this_type(
                buckets_,
                static_cast<cref<HasherType_>>(*this),
                static_cast<cref<KeyEqualsType_>>(*this),
                allocator(),
                _max_load_factor
            );
        }

        /**
         * Get a static empty bucket
         *
         * @author Julius
         * @date 04.10.2021
         *
         * @returns A ptr&lt;bucket_type&gt;
         */
        [[nodiscard]] static ptr<bucket_type> static_empty_bucket() noexcept {
            static bucket_type empty_bucket;
            return &empty_bucket;
        }

    private:
        FORCE_INLINE void rehash_impl(const size_type count_) {

            this_type next = deduce(count_);

            if (!_overflow.empty()) {
                next._overflow.swap(_overflow);
                next._element_count += next._overflow.size();

                for (cref<value_type> entry : next._overflow) {
                    const index_type idx = next.hash_to_bucket(hash_key(select_key(entry)));
                    next._buckets[idx].set_overflow(true);
                }
            }

            try {
                const bool use_hash = use_stored_hash(next.bucket_count());

                for (auto bi = _buckets.begin(); bi != _buckets.end(); ++bi) {

                    if (bi->empty())
                        continue;

                    const hash_type hk = use_hash ? bi->truncated_hash() : next.hash_key(select_key(bi->value()));
                    const index_type idx = next.hash_to_bucket(hk);

                    next.insert_value(idx, hk, _STD move(bi->value()));
                    erase_from_bucket(*bi, hash_to_bucket(hk));

                }
            } catch (...) {

                _overflow.swap(next._overflow);

                const bool use_hash = use_stored_hash(next.bucket_count());

                for (auto bi = next._buckets.begin(); bi != next._buckets.end(); ++bi) {

                    if (bi->empty())
                        continue;

                    const hash_type hk = use_hash ? bi->truncated_hash() : hash_key(select_key(bi->value()));
                    const index_type idx = hash_to_bucket(hk);

                    insert_value(idx, hk, _STD move(bi->value()));
                }

                throw;
            }

            next.swap(*this);
        }

    private:
        void erase_from_bucket(ref<bucket_type> bucket_, index_type idx_) noexcept {

            const size_type range = _STD distance(_buckets.data(), _STD addressof(bucket_));
            DEBUG_ASSERT(range >= idx_, "")

            bucket_.remove();
            _buckets[idx_].store_neighbor_shift(range - idx_);
            --_element_count;
        }

        overflow_iterator erase_from_overflow(overflow_const_iterator where_, index_type idx_) {

            auto it = _overflow.erase(where_);
            --_element_count;

            for (cref<value_type> entry : _overflow) {

                index_type eidx = hash_to_bucket(hash_key(select_key(entry)));

                if (eidx == idx_) {
                    return it;
                }
            }

            _buckets[idx_].set_overflow(false);
            return it;
        }

    private:
        /**
         * Implementation of try emplace
         *
         * @author Julius
         * @date 04.10.2021
         *
         * @param  value_ The value.
         *
         * @returns A pair&lt;iterator,bool&gt;
         */
        _STD pair<iterator, bool> try_emplace_impl(mref<value_type> value_) {

            const key_type& key = select_key(value_);
            const hash_type hk = hash_key(key);
            const index_type idx = hash_to_bucket(hk);

            /**
             * Check if already exists
             */
            auto it = find_impl(key, hk, _buckets_ptr + idx);

            if (it != end()) {
                return _STD make_pair(it, false);
            }

            return insert_value(
                idx,
                hk,
                _STD forward<value_type>(value_)
            );
        }

    private:
        _STD pair<iterator, bool> insert_impl(mref<value_type> value_) {

            const key_type& key = select_key(value_);
            const hash_type hk = hash_key(key);
            const index_type idx = hash_to_bucket(hk);

            auto it = find_impl(key, hk, _buckets_ptr + idx);

            if (it != end()) {
                return _STD make_pair(it, false);
            }

            return insert_value(idx, hk, _STD forward<value_type>(value_));
        }

        _STD pair<iterator, bool> insert_or_assign_impl(mref<value_type> value_) {

            auto it = try_emplace_impl(_STD forward<value_type>(value_));

            if (!it.second) {
                *it.first = _STD move(value_);
            }

            return it;
        }

    private:
        _STD pair<iterator, bool> insert_value(index_type idx_, hash_type hash_, mref<value_type> value_) {

            if ((_element_count - _overflow.size()) >= _max_load_threshold) {
                rehash(GrowthPolicyType_::next_bucket_count());
                idx_ = hash_to_bucket(hash_);
            }

            auto empty_idx = find_empty_bucket(idx_);
            if (empty_idx < _buckets.size()) {

                do {
                    /**
                     * Check if empty bucket is in range of neighborhood
                     */
                    if (empty_idx - idx_ < NeighborhoodSize_) {
                        auto it = insert_in_bucket(empty_idx, idx_, hash_, _STD forward<value_type>(value_));
                        return _STD make_pair(
                            iterator(it, _buckets.end(), _overflow.begin()),
                            true
                        );
                    }
                } while (move_empty_bucket_lower(empty_idx));
            }

            /**
             * Check whether
             *  -> current load is overall load or just peak cause of hash collision ( bad hashing )
             *  -> grow of container will change neighborhood
             */
            if (size() < _min_load_threshold || !will_change_neighbor(idx_)) {

                auto it = insert_in_overflow(idx_, _STD forward<value_type>(value_));
                return _STD make_pair(
                    iterator { _buckets.end(), _buckets.end(), it },
                    true
                );
            }

            /**
             * Grow container
             */
            rehash(GrowthPolicyType_::next_bucket_count());
            idx_ = hash_to_bucket(hash_);

            /**
             * Repeat recursive until insert succeeds
             */
            return insert_value(idx_, hash_, _STD forward<value_type>(value_));
        }

        /**
         * Check whether rehashing and growing will change neighborhood of given indexed bucket
         *
         * @author Julius
         * @date 04.10.2021
         *
         * @param  idx_ Zero-based index of the.
         *
         * @returns True if it succeeds, false if it fails.
         */
        bool will_change_neighbor(const index_type idx_) const noexcept {

            /**
             * Get policy constraints for next container size
             */
            size_type extended = GrowthPolicyType_::next_bucket_count();
            GrowthPolicyType_ policy { extended };

            /**
             * Check whether use stored hashes
             */
            const bool stored = use_stored_hash(extended);

            /**
             * Get upper boundary of neighborhood
             */
            const index_type upper = idx_ + NeighborhoodSize_;

            for (index_type l = idx_; l < _buckets.size() && l < upper; ++l) {
                /**
                 * Get hash value of current checked bucket
                 */
                const hash_type sh = stored ?
                                         _buckets_ptr[l].truncated_hash() :
                                         hash_key(select_key(_buckets_ptr[l].value()));

                /**
                 * Check if bucket of neighborhood will get moved
                 */
                if (hash_to_bucket(sh) != policy.bucket_idx(sh)) {
                    return true;
                }
            }

            /**
             * Growth won't change buckets neighborhood
             */
            return false;
        }

        /**
         * Searches for the first empty bucket
         *
         * @author Julius
         * @date 04.10.2021
         *
         * @param  first_ The first.
         *
         * @returns The found empty bucket.
         */
        index_type find_empty_bucket(index_type first_) const noexcept {

            const index_type limit = _STD min<index_type>(first_ + NeighborhoodSize_, _buckets.size());

            for (; first_ < limit; ++first_) {
                if (_buckets_ptr[first_].empty()) {
                    return first_;
                }
            }

            return _buckets.size();
        }

        /**
         * Move empty buckets to lower index
         *
         * @author Julius
         * @date 04.10.2021
         *
         * @param  empty_ The empty.
         *
         * @returns True if it succeeds, false if it fails.
         */
        [[nodiscard]] FORCE_INLINE bool move_empty_bucket_lower(IN OUT ref<index_type> empty_) {

            const index_type start = empty_ - NeighborhoodSize_ + 1u;

            for (index_type idx = start; idx < empty_; ++idx) {

                auto bitmap = _buckets_ptr[idx].neighbor();
                index_type sidx = idx;

                while (bitmap != 0 && sidx < empty_) {
                    if ((bitmap & 1) == 1) {

                        _buckets_ptr[sidx].swap_into_empty(_buckets_ptr[empty_]);
                        _buckets_ptr[idx].store_neighbor_shift(empty_ - idx);
                        _buckets_ptr[idx].store_neighbor_shift(sidx - idx);

                        empty_ = sidx;
                        return true;
                    }

                    ++sidx;
                    bitmap >>= 1;
                }

            }

            return false;
        }

    private:
        bucket_iterator insert_in_bucket(
            index_type empty_,
            index_type idx_,
            hash_type hash_,
            mref<value_type> value_
        ) {

            _buckets_ptr[empty_].store(hash_, _STD forward<value_type>(value_));
            _buckets_ptr[idx_].store_neighbor_shift(empty_ - idx_);

            ++_element_count;

            return _buckets.begin() + empty_;
        }

        overflow_iterator insert_in_overflow(index_type idx_, mref<value_type> value_) {

            auto it = _overflow.insert(_overflow.end(), _STD forward<value_type>(value_));

            _buckets_ptr[idx_].set_overflow(true);
            ++_element_count;

            return it;
        }

    private:
        [[nodiscard]] size_type count_impl(cref<key_type> key_, hash_type hash_, const ptr<bucket_type> bucket_) const {

            if (find_in_buckts(key_, hash_, bucket_) != nullptr) {
                return 1;
            }

            if (bucket_->overflow() && find_in_overflow(key_) != _overflow.cend()) {
                return 1;
            }

            return 0;
        }

    private:
        /**
         * Searches for a entry by the given key and hash
         *
         * @author Julius
         * @date 04.10.2021
         *
         * @param  key_ The key.
         * @param  hash_ The hash.
         * @param  bucket_ The bucket.
         *
         * @returns The found implementation.
         */
        [[nodiscard]] const_iterator find_impl(
            cref<key_type> key_,
            hash_type hash_,
            const ptr<bucket_type> bucket_
        ) const noexcept {

            ptr<bucket_type> found = find_in_buckets(key_, hash_, bucket_);

            if (found != nullptr) {
                return const_iterator {
                    _buckets.cbegin() + _STD distance(_buckets.data(), found),
                    _buckets.cend(),
                    _overflow.cbegin()
                };
            }

            if (!bucket_->overflow()) {
                return cend();
            }

            return const_iterator {
                _buckets.cend(),
                _buckets.cend(),
                find_in_overflow(key_)
            };
        }

        /**
         * Searches for a entry by the given key and hash
         *
         * @author Julius
         * @date 04.10.2021
         *
         * @param  key_ The key.
         * @param  hash_ The hash.
         * @param  bucket_ The bucket.
         *
         * @returns The found implementation.
         */
        [[nodiscard]] iterator find_impl(
            cref<key_type> key_,
            hash_type hash_,
            const ptr<bucket_type> bucket_
        ) noexcept {

            ptr<bucket_type> found = find_in_buckets(key_, hash_, bucket_);

            if (found != nullptr) {
                return iterator {
                    _buckets.begin() + _STD distance(_buckets.data(), found),
                    _buckets.end(),
                    _overflow.begin()
                };
            }

            if (!bucket_->overflow()) {
                return end();
            }

            return iterator {
                _buckets.end(),
                _buckets.end(),
                find_in_overflow(key_)
            };
        }

        const ptr<bucket_type> find_value_impl(
            cref<key_type> key_,
            hash_type hash_,
            const ptr<bucket_type> bucket_
        ) const noexcept {

            const auto* found = find_in_buckets(key_, hash_, bucket_);

            if (found != nullptr) {
                return found;
            }

            if (bucket_->overflow()) {

                auto it = find_in_overflow(key_);

                if (it != _overflow.end()) {
                    return _STD addressof(*it);
                }
            }

            return nullptr;
        }

        ptr<bucket_type> find_value_impl(cref<key_type> key_, hash_type hash_, const ptr<bucket_type> bucket_) {
            return const_cast<ptr<bucket_type>>(static_cast<const this_type*>(this)->find_value_impl(
                key_,
                hash_,
                bucket_
            ));
        }

        /**
         * Searches for the bucket with given key
         *
         * @author Julius
         * @date 04.10.2021
         *
         * @param  key_ The key.
         * @param  hash_ The hash.
         * @param  bucket_ The bucket.
         *
         * @returns The found in buckets.
         */
        [[nodiscard]] const ptr<bucket_type> find_in_buckets(
            cref<key_type> key_,
            hash_type hash_,
            ptr<bucket_type> bucket_
        ) const noexcept {

            auto nbm = bucket_->neighbor();

            /**
             * Check for value in addressed bucket and neighborhood
             */
            while (nbm != 0) {
                /**
                 * Check if current bit has stored value
                 */
                if ((nbm & 1) == 1) {
                    /**
                     * Check whether current bucket does contain value
                     */
                    if ((!StoreHash_ || bucket_->hash_equal(hash_)) && cmp(select_key(bucket_->value()), key_)) {
                        return bucket_;
                    }
                }

                /**
                 * Shift current bucket to next
                 */
                ++bucket_;

                /**
                 * Pop first neighbor
                 */
                nbm >>= 1;
            }

            return nullptr;
        }

        [[nodiscard]] ptr<bucket_type> find_in_buckets(
            cref<key_type> key_,
            hash_type hash_,
            ptr<bucket_type> bucket_
        ) noexcept {
            const auto* found = static_cast<const this_type*>(this)->find_in_buckets(key_, hash_, bucket_);
            return const_cast<ptr<bucket_type>>(found);
        }

        /**
         * Searches for the bucket with given key in overflow
         *
         * @author Julius
         * @date 04.10.2021
         *
         * @param  key_ The key.
         *
         * @returns The found in overflow.
         */
        [[nodiscard]] overflow_const_iterator find_in_overflow(cref<key_type> key_) const noexcept {

            return _STD find_if(
                _overflow.begin(),
                _overflow.end(),
                [&](cref<value_type> value_) {
                    return cmp(key_, select_key(value_));
                }
            );
        }

        /**
         * Searches for the bucket with given key in overflow
         *
         * @author Julius
         * @date 04.10.2021
         *
         * @param  key_ The key.
         *
         * @returns The found in overflow.
         */
        [[nodiscard]] overflow_iterator find_in_overflow(cref<key_type> key_) noexcept {

            return _STD find_if(
                _overflow.begin(),
                _overflow.end(),
                [&](cref<value_type> value_) {
                    return cmp(key_, select_key(value_));
                }
            );
        }
    };
}
