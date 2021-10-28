#pragma once

#include <stdexcept>
#include <unordered_set>
#include <Engine.Common/Collection/List.hpp>

#include "Component.hpp"
#include "Traits.hpp"

namespace ember::engine::ecs {

    template<typename Ty>
    FORCE_INLINE Ty *construct_inplace(void *destination_) {
        return new(destination_) Ty;
    }

    template<typename Ty>
    FORCE_INLINE Ty *construct_inplace(void *destination_, const Ty &source_) {
        return new(destination_) Ty(source_);
    }

    template<typename Ty>
    FORCE_INLINE Ty *construct_inplace(void *destination_, Ty &&source_) {
        return new(destination_) Ty(_STD forward<Ty>(source_));
    }

    template<typename Ty>
    FORCE_INLINE void destruct_inplace(Ty *destination_) {
        delete destination_;
    }

    /**
     * Forward declaration for hybrid_storage
     *
     * @author Julius
     * @date 23.10.2020
     *
     * @tparam ComponentType Type of the component type.
     */
    template<IsComponent ComponentType>
    class hybrid_storage;

    /**
     * A hybrid storage page. This class cannot be inherited..
     *
     *	A hybrid storage page will hold separated key-value pairs while using fixed size memory blocks.
     *	A memory block will be divided into a key and value section. Every section can hold up on to `page_size [div] (key_size + value_size)` elements.
     *
     * @author Julius
     * @date 23.10.2020
     *
     * @tparam ValueType Type of the component type.
     * @tparam KeyType Type of the entity identifier type.
     */
    template<IsComponent ValueType, typename KeyType, typename IndexType = size_t>
    class hybrid_storage_page final {

        friend class hybrid_storage<ValueType>;

    public:
        using this_type = hybrid_storage_page<ValueType, KeyType, IndexType>;
        using this_reference_type = this_type &;
        using this_const_reference_type = const this_type &;

        using index_type = IndexType;
        using key_type = KeyType;
        using value_type = ValueType;

    public:
        inline constexpr static size_t key_size = sizeof(key_type);
        inline constexpr static size_t value_size = sizeof(value_type);

        inline constexpr static size_t page_size = 32786;// 32kB Memory Blocks
        inline constexpr static size_t per_page = page_size / (key_size + value_size);

    public:
        /**
         * Move Constructor
         *
         * @author Julius
         * @date 23.10.2020
         *
         * @param [in,out] other_ The other.
         */
        hybrid_storage_page(this_type &&other_) noexcept:
                _mem(_STD exchange(other_._mem, nullptr)),
                _keys(_STD exchange(other_._keys, nullptr)),
                _values(_STD exchange(other_._values, nullptr)),
                _seq(_STD move(other_._seq)) {}

        /**
         * Copy Constructor
         *
         * @author Julius
         * @date 23.10.2020
         *
         * @param  other_ The other.
         */
        hybrid_storage_page(this_const_reference_type other_) = delete;

        /**
         * Destructor
         *
         * @author Julius
         * @date 23.10.2020
         */
        ~hybrid_storage_page() noexcept {
            free(_mem);
        }

        /**
         * Determine if we can store
         *
         * @author Julius
         * @date 02.11.2020
         *
         * @returns True if we can store, false if not.
         */
        [[nodiscard]] bool can_store() const noexcept {
            return !_seq.empty();
        }

        /**
         * Emplaces a element by the given key
         *
         * @author Julius
         * @date 23.10.2020
         *
         * @param 		   key_ The key.
         * @param [in,out] value_ The value.
         * @param [in,out] idx_ Zero-based index of the.
         *
         * @returns True if it succeeds, false if it fails.
         */
        bool emplace(IN const key_type &key_, IN value_type &&value_, OUT index_type &idx_) {
            /**
             * Gets empty slot
             */
            idx_ = pop_slot();

            /**
             * Place the new key and value to local storage
             */
            place(idx_, key_, _STD forward<value_type>(value_));

            /**
             * return if idx_ is valid
             */
            return idx_ > per_page;
        }

        /**
         * Gets a pair&lt;const key type&amp;,const value type&amp;&gt; using the given index
         *
         * @author Julius
         * @date 25.10.2020
         *
         * @param  idx_ The Index to get.
         *
         * @returns A pair&lt;const key_type&amp;,const value_type&amp;&gt;
         */
        [[nodiscard]] _STD pair<const key_type &, const value_type &> get(const index_type &idx_) const {
            return {
                    _keys[idx_],
                    _values[idx_]
            };
        }

        /**
         * Gets a pair&lt;const key type&amp;,value type&amp;&gt; using the given index
         *
         * @author Julius
         * @date 25.10.2020
         *
         * @param  idx_ The Index to get.
         *
         * @returns A pair&lt;const key_type&amp;,value_type&amp;&gt;
         */
        [[nodiscard]] _STD pair<const key_type &, value_type &> get(const index_type &idx_) {
            return {
                    _keys[idx_],
                    _values[idx_]
            };
        }

        /**
         * Gets a key
         *
         * @author Julius
         * @date 25.10.2020
         *
         * @param  idx_ Zero-based index of the.
         *
         * @returns The key.
         */
        [[nodiscard]] const key_type &get_key(const index_type &idx_) const {
            return _keys[idx_];
        }

        /**
         * Gets a key
         *
         * @author Julius
         * @date 25.10.2020
         *
         * @param  idx_ Zero-based index of the.
         *
         * @returns The key.
         */
        [[nodiscard]] key_type &get_key(const index_type &idx_) {
            return _keys[idx_];
        }

        /**
         * Gets a value
         *
         * @author Julius
         * @date 25.10.2020
         *
         * @param  idx_ Zero-based index of the.
         *
         * @returns The value.
         */
        [[nodiscard]] const value_type &get_value(const index_type &idx_) const {
            return _values[idx_];
        }

        /**
         * Gets a value
         *
         * @author Julius
         * @date 25.10.2020
         *
         * @param  idx_ Zero-based index of the.
         *
         * @returns The value.
         */
        [[nodiscard]] value_type &get_value(const index_type &idx_) {
            return _values[idx_];
        }

        /**
         * Inserts a element by the given key
         *
         * @author Julius
         * @date 23.10.2020
         *
         * @param 		   idx_ Zero-based index of the.
         * @param 		   key_ The key.
         * @param [in,out] value_ The value.
         */
        void insert(const index_type &idx_, const key_type &key_, IN value_type &&value_) {
            /**
             * Check if key exists
             */
            if (_keys[idx_]) {
                /**
                 * If index in range, override stored element
                 */
                replace(idx_, key_, _STD forward<value_type>(value_));
            } else {
                /**
                 * reserve slot from sequence
                 */
                pop_slot(idx_);

                /**
                 * Place the new key and value to local storage
                 */
                place(idx_, key_, _STD forward<value_type>(value_));
            }
        }

        /**
         * Erases the given key and related element
         *
         * @author Julius
         * @date 24.10.2020
         *
         * @param  idx_ The key.
         */
        void erase(const index_type &idx_) {
            // destruct_inplace<key_type>(&_keys[idx_]);
            _keys[idx_] = invalid_entity_guid;
            destruct_inplace<value_type>(&_values[idx_]);

            /**
             * Find valid minimal sequence where index can merge
             */
            auto fi = _STD find_if(_seq.rbegin(), _seq.rend(), [&idx_](const page_sequence &entry_) {
                return entry_.mergable(idx_);
            });

            /**
             * If no mergable sequence, create a new one
             */
            if (fi == _seq.rend()) {
                _seq.push_back({idx_, idx_});
                _STD sort(_seq.begin(), _seq.end(), page_sequence_range_comparator());
            }

            /**
             * Find valid maximal sequence where index can merge
             */
            auto ri = _STD find_if(_seq.begin(), _seq.end(), [&idx_](const page_sequence &entry_) {
                return entry_.mergable(idx_);
            });

            /**
             * If only one sequence can merge idx_
             */
            if (*fi == *ri) {
                /**
                 * Merge the index to the only available sequence
                 */
                fi->merge(idx_);
            } else {
                /**
                 * Merge the index to the smaller sequence
                 */
                fi->merge(idx_);

                /**
                 * Copy smaller sequence to hold value to temporary
                 */
                auto v = *fi;

                /**
                 * Merge the bigger sequence with the smaller one
                 */
                v.merge(*ri);

                /**
                 * Erase the forward iterator and reverse iterator while there reference is stable
                 */
                _seq.erase(fi);
                _seq.erase(ri);

                /**
                 * Push the new sequence to list
                 */
                _seq.push_back(v);
                _STD sort(_seq.begin(), _seq.end(), page_sequence_range_comparator());
            }
        }

        /**
         * A hybrid value iterator. This class cannot be inherited..
         *
         * @author Julius
         * @date 25.10.2020
         */
        class hybrid_value_iterator final {
        public:
            hybrid_value_iterator() noexcept:
                    _ptr(nullptr) {}

            hybrid_value_iterator(IN value_type *ptr_) noexcept:
                    _ptr(ptr_) {}

            hybrid_value_iterator(IN value_type *ptr_, const size_t idx_) noexcept:
                    _ptr(&ptr_[idx_]) {}

            hybrid_value_iterator(IN hybrid_value_iterator &&other_) noexcept:
                    _ptr(_STD exchange(other_._ptr, nullptr)) {}

            hybrid_value_iterator(IN const hybrid_value_iterator &other_) noexcept:
                    _ptr(other_._ptr) {}

            hybrid_value_iterator &operator++() noexcept {
                return ++_ptr, *this;
            }

            hybrid_value_iterator &operator--() noexcept {
                return --_ptr, *this;
            }

            hybrid_value_iterator operator++(int) noexcept {
                const auto last = *this;
                return ++_ptr, last;
            }

            hybrid_value_iterator operator--(int) noexcept {
                const auto last = *this;
                return --_ptr, last;
            }

            [[nodiscard]] hybrid_value_iterator operator+(const index_type &range_) const noexcept {
                return hybrid_value_iterator{(_ptr + range_)};
            }

            [[nodiscard]] hybrid_value_iterator operator-(const index_type &range_) const noexcept {
                return hybrid_value_iterator{(_ptr - range_)};
            }

            hybrid_value_iterator &operator+=(const index_type &range_) const noexcept {
                return (_ptr += range_), *this;
            }

            hybrid_value_iterator &operator-=(const index_type &range_) const noexcept {
                return (_ptr -= range_), *this;
            }

            [[nodiscard]] bool operator==(const hybrid_value_iterator &other_) const noexcept {
                return other_._ptr == _ptr;
            }

            [[nodiscard]] bool operator!=(const hybrid_value_iterator &other_) const noexcept {
                return other_._ptr != _ptr;
            }

            [[nodiscard]] bool operator<(const hybrid_value_iterator &other_) const noexcept {
                return other_._ptr < _ptr;
            }

            [[nodiscard]] bool operator>(const hybrid_value_iterator &other_) const noexcept {
                return other_._ptr > _ptr;
            }

            [[nodiscard]] bool operator<=(const hybrid_value_iterator &other_) const noexcept {
                return other_._ptr <= _ptr;
            }

            [[nodiscard]] bool operator>=(const hybrid_value_iterator &other_) const noexcept {
                return other_._ptr >= _ptr;
            }

            hybrid_value_iterator &operator=(IN hybrid_value_iterator &&other_) noexcept {
                return (_ptr = _STD exchange(other_._ptr, nullptr)), *this;
            }

            hybrid_value_iterator &operator=(const hybrid_value_iterator &other_) noexcept {
                if (this == &other_) {
                    return *this;
                }
                return (_ptr = other_._ptr), *this;
            }

            const value_type &operator*() const {
                return *_ptr;
            }

            value_type &operator*() {
                return *_ptr;
            }

            const value_type *operator->() const {
                return _ptr;
            }

            value_type *operator->() {
                return _ptr;
            }

        private:
            value_type *_ptr;
        };

        /**
         * A hybrid key value iterator. This class cannot be inherited..
         *
         * @author Julius
         * @date 25.10.2020
         */
        template<bool Const>
        class hybrid_key_value_iterator final {
        public:
            using iterator_key_type = const key_type;
            using iterator_value_type = typename _STD conditional<Const, const value_type, value_type>::type;

            /**
             * Default constructor
             *
             * @author Julius
             * @date 02.11.2020
             */
            hybrid_key_value_iterator() noexcept:
                    _keys(nullptr),
                    _values(nullptr) {}

            /**
             * Constructor
             *
             * @author Julius
             * @date 02.11.2020
             *
             * @param [in,out] keys_ If non-null, the keys.
             * @param [in,out] values_ If non-null, the values.
             */
            hybrid_key_value_iterator(IN key_type *keys_, IN value_type *values_) noexcept:
                    _keys(keys_),
                    _values(values_) {}

            /**
             * Constructor
             *
             * @author Julius
             * @date 02.11.2020
             *
             * @param [in,out] keys_ If non-null, the keys.
             * @param [in,out] values_ If non-null, the values.
             * @param 		   idx_ Zero-based index of the.
             */
            hybrid_key_value_iterator(IN key_type *keys_, IN value_type *values_, const index_type idx_) noexcept:
                    _keys(&keys_[idx_]),
                    _values(&values_[idx_]) {}

            /**
             * Move Constructor
             *
             * @author Julius
             * @date 02.11.2020
             *
             * @param [in,out] other_ The other.
             */
            hybrid_key_value_iterator(IN hybrid_key_value_iterator &&other_) noexcept:
                    _keys(_STD exchange(other_._keys, nullptr)),
                    _values(_STD exchange(other_._values, nullptr)) {}

            /**
             * Copy Constructor
             *
             * @author Julius
             * @date 02.11.2020
             *
             * @param  other_ The other.
             */
            hybrid_key_value_iterator(IN const hybrid_key_value_iterator &other_) :
                    _keys(other_._keys),
                    _values(other_._values) {}

            hybrid_key_value_iterator &operator++() noexcept {
                return ++_keys, ++_values, *this;
            }

            hybrid_key_value_iterator &operator--() noexcept {
                return --_keys, --_values, *this;
            }

            hybrid_key_value_iterator operator++(int) noexcept {
                const auto last = *this;
                return ++_keys, ++_values, *this;
            }

            hybrid_key_value_iterator operator--(int) noexcept {
                const auto last = *this;
                return --_keys, --_values, *this;
            }

            [[nodiscard]] hybrid_key_value_iterator operator+(const index_type &range_) const noexcept {
                return hybrid_key_value_iterator{_keys + range_, _values + range_};
            }

            [[nodiscard]] hybrid_key_value_iterator operator-(const index_type &range_) const noexcept {
                return hybrid_key_value_iterator{_keys - range_, _values - range_};
            }

            hybrid_key_value_iterator &operator+=(const index_type &range_) noexcept {
                return (_keys += range_), (_values += range_), *this;
            }

            hybrid_key_value_iterator &operator-=(const index_type &range_) noexcept {
                return (_keys -= range_), (_values -= range_), *this;
            }

            [[nodiscard]] bool operator==(const hybrid_key_value_iterator &other_) const noexcept {
                return other_._keys == _keys && other_._values == _values;
            }

            [[nodiscard]] bool operator!=(const hybrid_key_value_iterator &other_) const noexcept {
                return other_._keys != _keys || other_._values != _values;
            }

            [[nodiscard]] bool operator<(const hybrid_key_value_iterator &other_) const noexcept {
                return other_._keys < _keys && other_._values < _values;
            }

            [[nodiscard]] bool operator>(const hybrid_key_value_iterator &other_) const noexcept {
                return other_._keys > _keys && other_._values > _values;
            }

            [[nodiscard]] bool operator<=(const hybrid_key_value_iterator &other_) const noexcept {
                return other_._keys <= _keys && other_._values <= _values;
            }

            [[nodiscard]] bool operator>=(const hybrid_key_value_iterator &other_) const noexcept {
                return other_._keys >= _keys && other_._values >= _values;
            }

            hybrid_key_value_iterator &operator=(IN hybrid_key_value_iterator &&other_) noexcept {
                _keys = _STD exchange(other_._keys, nullptr);
                _values = _STD exchange(other_._values, nullptr);
                return *this;
            }

            hybrid_key_value_iterator &operator=(const hybrid_key_value_iterator &other_) noexcept {
                if (this == &other_) {
                    return *this;
                }

                _keys = other_._keys;
                _values = other_._values;
                return *this;
            }

            /**
             * Indirection operator
             *
             * @author Julius
             * @date 31.10.2020
             *
             * @returns The result of the operation.
             */
            [[nodiscard]] _STD pair<iterator_key_type &, iterator_value_type &> operator*() const {
                return _STD pair<iterator_key_type &, iterator_value_type &>{
                        *_keys,
                        *_values
                };
            }

            /**
             * Indirection operator
             *
             * @author Julius
             * @date 31.10.2020
             *
             * @returns The result of the operation.
             */
            [[nodiscard]] _STD pair<iterator_key_type &, iterator_value_type &> operator*() {
                return _STD pair<iterator_key_type &, iterator_value_type &>{
                        *_keys,
                        *_values
                };
            }

            /**
             * Member dereference operator
             *
             * @author Julius
             * @date 31.10.2020
             *
             * @returns The dereferenced object.
             */
            [[nodiscard]] _STD pair<iterator_key_type &, iterator_value_type &> operator->() const {
                return _STD pair<iterator_key_type &, iterator_value_type &>{
                        *_keys,
                        *_values
                };
            }

            /**
             * Member dereference operator
             *
             * @author Julius
             * @date 31.10.2020
             *
             * @returns The dereferenced object.
             */
            [[nodiscard]] _STD pair<iterator_key_type &, iterator_value_type &> operator->() {
                return _STD pair<iterator_key_type &, iterator_value_type &>{
                        *_keys,
                        *_values
                };
            }

            /**
             * Returns the distance of this and other_
             *
             * @author Julius
             * @date 31.10.2020
             *
             * @param  other_ The other.
             *
             * @returns A size_t.
             */
            [[nodiscard]] _STD size_t diff(const this_type &other_) const {
                return MAX(_keys, other_._keys) - MIN(_keys, other_._keys);
            }

        private:
            key_type *_keys;
            value_type *_values;
        };

        using iterator = hybrid_key_value_iterator<false>;
        using const_iterator = hybrid_key_value_iterator<true>;

        /**
         * Gets the iterator to the begin
         *
         * @author Julius
         * @date 31.10.2020
         *
         * @returns An iterator.
         */
        [[nodiscard]] iterator begin() noexcept {
            return iterator{
                    _keys,
                    _values
            };
        }

        /**
         * Gets the iterator to the end
         *
         * @author Julius
         * @date 31.10.2020
         *
         * @returns An iterator.
         */
        [[nodiscard]] iterator end() noexcept {
            return iterator{
                    _keys,
                    _values,
                    this_type::per_page - 1u
            };
        }

        /**
         * Gets the constant iterator to the begin
         *
         * @author Julius
         * @date 31.10.2020
         *
         * @returns A const_iterator.
         */
        [[nodiscard]] const_iterator begin() const noexcept {
            return const_iterator{
                    _keys,
                    _values
            };
        }

        /**
         * Gets the constant iterator to the end
         *
         * @author Julius
         * @date 31.10.2020
         *
         * @returns A const_iterator.
         */
        [[nodiscard]] const_iterator end() const noexcept {
            return const_iterator{
                    _keys,
                    _values,
                    this_type::per_page - 1u
            };
        }

        /**
         * Gets the constant iterator to the begin
         *
         * @author Julius
         * @date 31.10.2020
         *
         * @returns A const_iterator.
         */
        [[nodiscard]] const_iterator cbegin() const noexcept {
            return const_iterator{
                    _keys,
                    _values
            };
        }

        /**
         * Gets the constant iterator to the end
         *
         * @author Julius
         * @date 31.10.2020
         *
         * @returns A const_iterator.
         */
        [[nodiscard]] const_iterator cend() const noexcept {
            return const_iterator{
                    _keys,
                    _values,
                    this_type::per_page - 1u
            };
        }

        /**
         * A page sequence.
         *
         * @author Julius
         * @date 25.10.2020
         */
        struct page_sequence {
            index_type begin;
            index_type end;

            /**
             * Merges the given index into this sequence
             *
             * @author Julius
             * @date 25.10.2020
             *
             * @param  index_ Zero-based index of the.
             *
             * @returns A reference to a page_sequence.
             */
            page_sequence &merge(const index_type &index_) {
                return index_ < begin ? begin-- : end++, *this;
            }

            /**
             * Merges the given sequence and this to continuous sequence
             *
             * @author Julius
             * @date 25.10.2020
             *
             * @param  other_ The other.
             *
             * @returns A reference to a page_sequence.
             */
            page_sequence &merge(const page_sequence &other_) {
                begin = MIN(begin, other_.begin);
                end = MAX(end, other_.end);
                return *this;
            }

            /**
             * Check whether index can be merge into sequence
             *
             * @author Julius
             * @date 25.10.2020
             *
             * @param  index_ Zero-based index of the.
             *
             * @returns True if it succeeds, false if it fails.
             */
            [[nodiscard]] bool mergable(const index_type &index_) const {
                return (end + 1u <= index_) || (begin - 1u >= index_);
            }

            /**
             * Check whether distinct sequence can be merge into this
             *
             * @author Julius
             * @date 25.10.2020
             *
             * @param  other_ The other.
             *
             * @returns True if it succeeds, false if it fails.
             */
            [[nodiscard]] bool mergable(const page_sequence &other_) const {
                return (end + 1u <= other_.begin) || (begin - 1u >= other_.end);
            }

            [[nodiscard]] bool valid() const {
                return begin <= end;
            }

            /**
             * Return the size of elements described by this sequence.
             *	If begin and end are equal, only one element is described
             *
             * @author Julius
             * @date 25.10.2020
             *
             * @returns An index_type.
             */
            [[nodiscard]] index_type size() const {
                return (end + 1u) - begin;
            }

            /**
             * Query if this contains the given idx_
             *
             * @author Julius
             * @date 25.10.2020
             *
             * @param  idx_ The const index_type&amp; to test for containment.
             *
             * @returns True if the object is in this collection, false if not.
             */
            [[nodiscard]] bool contains(const index_type &idx_) const {
                return idx_ >= begin && idx_ <= end;
            }

            /**
             * Will increment the begin index
             *
             * @author Julius
             * @date 30.10.2020
             *
             * @returns The result of the operation.
             */
            page_sequence &operator++() noexcept {
                return ++begin, *this;
            }

            /**
             * Will decrement the begin index
             *
             * @author Julius
             * @date 30.10.2020
             *
             * @returns The result of the operation.
             */
            page_sequence &operator--() noexcept {
                return --begin, *this;
            }

            /**
             * Will increment the end index
             *
             * @author Julius
             * @date 30.10.2020
             *
             * @returns The result of the operation.
             */
            page_sequence &operator++(int) noexcept {
                return ++end, *this;
            }

            /**
             * Will decrement the end index
             *
             * @author Julius
             * @date 30.10.2020
             *
             * @returns The result of the operation.
             */
            page_sequence &operator--(int) noexcept {
                return --end, *this;
            }

            /**
             * Removes and returns the front element
             *
             * @author Julius
             * @date 25.10.2020
             *
             * @returns An index_type.
             */
            [[nodiscard]] index_type pop_front() {
                const auto idx = begin;
                return ++begin, idx;
            }

            /**
             * Removes and returns the back element
             *
             * @author Julius
             * @date 25.10.2020
             *
             * @returns An index_type.
             */
            [[nodiscard]] index_type pop_back() {
                const auto idx = end;
                return --end, idx;
            }

            /**
             * Resects the sequence while excluding the given value
             *
             * @author Julius
             * @date 25.10.2020
             *
             * @param  exclude_ The exclude.
             *
             * @returns A pair<page_sequence ~ lower,page_sequence ~ upper>
             */
            [[nodiscard]] _STD pair<page_sequence, page_sequence> resect(const index_type &exclude_) const {
                return {
                        {begin,         exclude_ - 1u},
                        {exclude_ + 1u, end}
                };
            }
        };

        /**
         * A page sequence range comparator.
         *	Comparator for will return true for sequence with bigger range
         *
         * @author Julius
         * @date 25.10.2020
         */
        struct page_sequence_range_comparator {
            bool operator()(const page_sequence &left_, const page_sequence &right_) {
                return left_.size() > right_.size();
            }
        };

    protected:
        /**
         * Default constructor
         *
         * @author Julius
         * @date 23.10.2020
         */
        explicit hybrid_storage_page() noexcept:
                _mem(allocate_safe()),
                _keys(static_cast<KeyType *>(static_cast<void *>(static_cast<ValueType *>(_mem) + per_page))),
                _values(static_cast<ValueType *>(_mem)),
                _seq(vector<page_sequence>({page_sequence{0, per_page}})) {}

    private:
        /**
         * The managed and containing memory block
         */
        void *_mem;

        key_type *_keys;// _mem[per_page * sizeof(value_type)]
        value_type *_values;// _mem[0]

        vector<page_sequence> _seq;

        /**
         * Pops the slot
         *
         * @author Julius
         * @date 25.10.2020
         *
         * @returns An index_type.
         */
        [[nodiscard]] FORCE_INLINE index_type pop_slot() {
            /**
             * Check whether sequence is available
             */
            if (_seq.empty()) {
                constexpr auto invalid = static_cast<index_type>(~0);
                return invalid;
            }

            /**
             * Get back of sorted list ( smallest element )
             */
            auto &s = _seq.back();

            /**
             * Get front index and remove from sequence
             */
            const auto idx = s.pop_front();

            /**
             * If sequence is invalid after reducing, pop from sequence collection
             */
            if (!s.valid()) {
                _seq.pop_back();
            }

            return idx;
        }

        /**
         * Pops the slot described by idx_
         *
         * @author Julius
         * @date 25.10.2020
         *
         * @param  idx_ Zero-based index of the.
         */
        FORCE_INLINE void pop_slot(const index_type &idx_) {
            /**
             * Find sequence
             */
            auto s = _STD find_if(_seq.rbegin(), _seq.rend(), [&idx_](const page_sequence &entry_) {
                return entry_.contains(idx_);
            });

            /**
             * Check whether idx_ containing sequence was found
             */
            if (s == _seq.end()) {
                return;
            }

            /**
             * 
             */
            if (s->begin == idx_) {
                /**
                 * If idx_ is at the begin, pre-decrement so front will reduce
                 */
                --(*s);

                if (!s->valid()) {
                    _seq.erase(s);
                }
            } else if (s->end == idx_) {
                /**
                 * If idx_ is at the end, post-decrement so back will reduce
                 */
                (*s)--;

                if (!s->valid()) {
                    _seq.erase(s);
                }
            } else {
                /**
                 * If idx_ is between, splice sequence and exclude idx_
                 */
                auto ul = s.resect(idx_);

                /**
                 * Erase old element while iterator is valid reference
                 */
                _seq.erase(s);

                /**
                 * Push new sequences to list
                 */
                _seq.push_back(ul.first);
                _seq.push_back(ul.second);
                _STD sort(_seq.begin(), _seq.end(), page_sequence_range_comparator());
            }
        }

        /**
         * Emplaces a key value pair to the indexed storage
         *
         * @author Julius
         * @date 24.10.2020
         *
         * @param 		   idx_ Zero-based index of the.
         * @param 		   key_ The key.
         * @param [in,out] value_ The value.
         */
        FORCE_INLINE void place(const index_type &idx_, const key_type &key_, IN value_type &&value_) noexcept {
            _keys[idx_] = key_;
            construct_inplace<value_type>(&_values[idx_], _STD forward<value_type>(value_));
        }

        /**
         * Replaces the key value pair at the indexed location
         *
         * @author Julius
         * @date 24.10.2020
         *
         * @param 		   idx_ Zero-based index of the.
         * @param 		   key_ The key.
         * @param [in,out] value_ The value.
         */
        FORCE_INLINE void replace(const index_type &idx_, const key_type &key_, IN value_type &&value_) noexcept {
            _keys[idx_] = key_;
            destruct_inplace<value_type>(&_values[idx_]);
            construct_inplace<value_type>(&_values[idx_], _STD forward<value_type>(value_));
        }

        /**
         * Allocate memory block and guarantees empty key space
         *
         * @author Julius
         * @date 23.10.2020
         *
         * @returns Null if it fails, else a pointer to a void.
         */
        static void *allocate_safe() noexcept {
            void *page = malloc(page_size);

            /**
             * 
             */
            constexpr size_t key_length = key_size * per_page;

            auto *key_ptr = static_cast<KeyType *>(
                    static_cast<void *>(
                            static_cast<ValueType *>(page) + per_page
                    )
            );

            /**
             * 
             */
            memset(key_ptr, 0, key_length);

            /**
             * 
             */
            return page;
        }
    };

    /**
     * A hybrid storage. This class cannot be inherited..
     *
     * @author Julius
     * @date 23.10.2020
     *
     * @tparam ComponentType Type of the component type.
     */
    template<IsComponent ComponentType>
    class hybrid_storage final {
    public:
        using this_type = hybrid_storage<ComponentType>;

    private:
        // TODO: Hopscotch Hashing with robust set to store entity_guid with indirection_keys
        // TODO: Indexing with bitmask separation
        constexpr static uint64_t index_page_mask = 0xFFFFFFFF00000000;
        constexpr static uint64_t index_value_mask = 0x00000000FFFFFFFF;
        constexpr static uint8_t index_page_shift = 32u;

        using page_index_type = uint32_t;
        using value_index_type = uint32_t;
        using index_type = uint64_t;

        using storage_page_type = hybrid_storage_page<ComponentType, entity_guid, value_index_type>;

        /**
         * A storage unique identifier index pair.
         *
         * @author Julius
         * @date 30.10.2020
         */
        struct storage_guid_idx_pair {
            entity_guid guid;
            index_type idx;
        };

        /**
         * A unique identifier index hasher.
         *
         * @author Julius
         * @date 30.10.2020
         */
        struct guid_idx_hasher :
                private _STD hash<entity_guid> {
            /**
             * Function call operator
             *
             * @author Julius
             * @date 30.10.2020
             *
             * @param  value_ The value.
             *
             * @returns The result of the operation.
             */
            [[nodiscard]] _STD size_t operator()(const storage_guid_idx_pair &value_) const {
                return static_cast<const _STD hash<entity_guid> &>(*this)(value_.guid);
            }
        };

        /**
         * A unique identifier index equal.
         *
         * @author Julius
         * @date 30.10.2020
         */
        struct guid_idx_equal :
                private _STD equal_to<entity_guid> {
            /**
             * Function call operator
             *
             * @author Julius
             * @date 30.10.2020
             *
             * @param  left_ The left.
             * @param  right_ The right.
             *
             * @returns The result of the operation.
             */
            [[nodiscard]] bool operator()(const storage_guid_idx_pair &left_,
                                          const storage_guid_idx_pair &right_) const {
                return static_cast<const _STD equal_to<entity_guid> &>(*this)(left_.guid, right_.guid);
            }
        };

        /**
         * A unique identifier index less.
         *
         * @author Julius
         * @date 30.10.2020
         */
        struct guid_idx_less :
                private _STD less<entity_guid> {
            /**
             * Function call operator
             *
             * @author Julius
             * @date 30.10.2020
             *
             * @param  left_ The left.
             * @param  right_ The right.
             *
             * @returns The result of the operation.
             */
            [[nodiscard]] bool operator()(const storage_guid_idx_pair &left_,
                                          const storage_guid_idx_pair &right_) const {
                return static_cast<const _STD less<entity_guid> &>(*this)(left_.guid, right_.guid);
            }
        };

    public:
        using mapping_container = _STD unordered_set<storage_guid_idx_pair, guid_idx_hasher, guid_idx_equal>;
        using hash_type = size_t;

    private:
        /**
         * An iterator. This class cannot be inherited..
         *
         *	Currently implemented as ForwardIterator
         *
         * @author Julius
         * @date 30.10.2020
         *
         * @tparam Const Type of the constant.
         */
        template<bool Const>
        class storage_key_value_iterator final {
        private:
            using this_type = storage_key_value_iterator<Const>;

            using key_type = entity_guid;
            using value_type = ComponentType;

            using storage_page_type = typename _STD conditional<Const, const storage_page_type, storage_page_type>::type;
            using storage_page_iterator_type = typename storage_page_type::hybrid_key_value_iterator;

            using page_collection_type = vector<storage_page_type>;
            using page_iterator_type = typename _STD conditional<Const, typename page_collection_type::const_iterator,
                    typename page_collection_type::iterator>::type;

        public:
            using iterator_category = _STD forward_iterator_tag;

            /**
             * Default constructor
             *
             * @author Julius
             * @date 30.10.2020
             */
            storage_key_value_iterator() noexcept = default;

            /**
             * Destructor
             *
             * @author Julius
             * @date 30.10.2020
             */
            ~storage_key_value_iterator() = default;

            /**
             * Gets the page
             *
             * @author Julius
             * @date 30.10.2020
             *
             * @returns A reference to a storage_page_type.
             */
            [[nodiscard]] storage_page_type &page() const {
                return *_pageIterator;
            }

            /**
             * Gets the page
             *
             * @author Julius
             * @date 30.10.2020
             *
             * @returns A reference to a storage_page_type.
             */
            [[nodiscard]] storage_page_type &page() {
                return *_pageIterator;
            }

            /**
             * Gets the key
             *
             * @author Julius
             * @date 30.10.2020
             *
             * @returns The key.
             */
            [[nodiscard]] const key_type &get_key() const {
                return (*_pairIterator).first;
            }

            /**
             * Gets the key
             *
             * @author Julius
             * @date 30.10.2020
             *
             * @returns The key.
             */
            [[nodiscard]] const key_type &get_key() {
                return (*_pairIterator).first;
            }

            /**
             * Gets the value
             *
             * @author Julius
             * @date 30.10.2020
             *
             * @returns The value.
             */
            [[nodiscard]] const value_type &get_value() const {
                return (*_pairIterator).second;
            }

            /**
             * Gets the value
             *
             * @author Julius
             * @date 30.10.2020
             *
             * @returns The value.
             */
            [[nodiscard]] value_type &get_value() {
                return (*_pairIterator).second;
            }

            /**
             * Indirection operator
             *
             * @author Julius
             * @date 30.10.2020
             *
             * @returns The result of the operation.
             */
            _STD pair<const entity_guid &, const value_type &> operator*() const {
                return *_pairIterator;
            }

            /**
             * Indirection operator
             *
             * @author Julius
             * @date 30.10.2020
             *
             * @returns The result of the operation.
             */
            _STD pair<const entity_guid &, value_type &> operator*() {
                return *_pairIterator;
            }

            /**
             * Member dereference operator
             *
             * @author Julius
             * @date 30.10.2020
             *
             * @returns The dereferenced object.
             */
            _STD pair<const entity_guid &, const value_type &> operator->() const {
                return *_pairIterator;
            }

            /**
             * Member dereference operator
             *
             * @author Julius
             * @date 30.10.2020
             *
             * @returns The dereferenced object.
             */
            _STD pair<const entity_guid &, value_type &> operator->() {
                return *_pairIterator;
            }

            /**
             * Increment operator
             *
             * @author Julius
             * @date 30.10.2020
             *
             * @returns The result of the operation.
             */
            this_type &operator++() {
                if (++(_pairIterator) == _pairEndIterator) {
                    ++_pageIterator;

                    _pairIterator = (*_pageIterator).begin();
                    _pairEndIterator = (*_pageIterator).end();
                }

                return *this;

            }

            // TODO: this_type& operator--() {}

            /**
             * Post Increment operator
             *
             * @author Julius
             * @date 30.10.2020
             *
             * @returns The result of the operation.
             */
            this_type operator++(int) {
                auto origin = *this;
                return ++(*this), origin;
            }

            /*
            this_type operator--(int) {
                auto origin = *this;
                return --(*this), origin;
            }
             */

            /**
             * Addition operator
             *
             * @author Julius
             * @date 30.10.2020
             *
             * @param  range_ The range.
             *
             * @returns The result of the operation.
             */
            this_type operator+(const index_type range_) const {
                auto target{*this};
                target += range_;
                return target;
            }

            /*
            this_type operator-(const index_type range_) const {
                auto target { *this };
                target -= range_;
                return target;
            }
             */

            /**
             * Addition assignment operator
             *
             * @author Julius
             * @date 30.10.2020
             *
             * @param  range_ The range.
             *
             * @returns The result of the operation.
             */
            this_type &operator+=(const index_type range_) {
                /**
                 * Get total offset of on-page iterator plus requested range
                 */
                index_type diff = range_ + (storage_page_type::per_page - _pairIterator.diff(_pairEndIterator));

                /**
                 * Get amount of pages to jump
                 */
                const index_type pageRange = range_ % storage_page_type::per_page;

                /**
                 * Get left offset for on-page iterator
                 */
                diff -= pageRange * storage_page_type::per_page;

                /**
                 * Move page iterator
                 */
                _pageIterator += pageRange;

                /**
                 * Restore pair iterator from new page with left offset
                 */
                _pairIterator = (*_pageIterator).begin() += diff;
                _pairEndIterator = (*_pageIterator).end();

                return *this;
            }

            // TODO: this_type& operator-=(const index_type range_) { }

            /**
             * Equality operator
             *
             * @author Julius
             * @date 30.10.2020
             *
             * @param  left_ The first instance to compare.
             * @param  right_ The second instance to compare.
             *
             * @returns True if the parameters are considered equivalent.
             */
            friend bool operator==(const this_type &left_, const this_type &right_) {
                return left_._pairIterator == right_._pairIterator && left_._pageIterator == right_._pageIterator;
            }

            /**
             * Inequality operator
             *
             * @author Julius
             * @date 30.10.2020
             *
             * @param  left_ The first instance to compare.
             * @param  right_ The second instance to compare.
             *
             * @returns True if the parameters are not considered equivalent.
             */
            friend bool operator!=(const this_type &left_, const this_type &right_) {
                return left_._pairIterator != right_._pairIterator || left_._pageIterator != right_._pageIterator;
            }

        private:
            page_iterator_type _pageIterator;
            page_iterator_type _pageEndIterator;

            storage_page_iterator_type _pairIterator;
            storage_page_iterator_type _pairEndIterator;
        };

    public:
        using iterator = storage_key_value_iterator<false>;
        using const_iterator = storage_key_value_iterator<true>;

    public:
        /**
         * Default constructor
         *
         * @author Julius
         * @date 25.10.2020
         */
        hybrid_storage() :
                _pages(),
                _indirection() {}

        /**
         * Move Constructor
         *
         * @author Julius
         * @date 25.10.2020
         *
         * @param [in,out] other_ The other.
         */
        hybrid_storage(hybrid_storage &&other_) noexcept:
                _pages(_STD move(other_._pages)),
                _indirection(_STD move(other_._indirection)) {}

        /**
         * Explicitly deleted Copy Constructor
         *
         * @author Julius
         * @date 25.10.2020
         *
         * @param  other_ The other.
         */
        hybrid_storage(const hybrid_storage &other_) = delete;

        /**
         * Destructor
         *
         * @author Julius
         * @date 25.10.2020
         */
        ~hybrid_storage() = default;

        /**
         * Emplaces a component with the given unique identifier and returns stable reference
         *
         * @author Julius
         * @date 30.10.2020
         *
         * @param  guid_ Unique identifier.
         *
         * @returns A pair&lt;ComponentType&amp;,bool&gt;
         */
        [[nodiscard]] _STD pair<ComponentType *, bool> emplace(const entity_guid &guid_) {
            _STD pair<typename mapping_container::iterator, bool> er = _indirection.emplace(storage_guid_idx_pair{
                    guid_,
                    0
            });
            storage_guid_idx_pair &inp = const_cast<storage_guid_idx_pair &>(*er.first);

            /**
             * Check whether emplace at indirection failed
             */
            ComponentType *ptr;
            if (er.second == false) {
                /**
                 * If emplace failed, get reference of already existing element
                 */
                ptr = &_pages[unmask_page_index(inp.idx)].get_value(unmask_value_index(inp.idx));
            } else {
                /**
                 * If emplace succeeded, construct element in pages compressed storage
                 */
                const index_type cidx = pages_insert_pair(guid_, {});

                /**
                 * Store cidx to indirection entry
                 */
                inp.idx = cidx;

                /**
                 * Get reference to existing element
                 */
                ptr = &_pages[unmask_page_index(cidx)].get_value(unmask_value_index(cidx));
            }

            return _STD make_pair(ptr, er.second);
        }

        /**
         * Emplaces a move constructed component by the given guid_
         *
         * @author Julius
         * @date 02.11.2020
         *
         * @param 		   guid_ Unique identifier.
         * @param [in,out] component_ The component.
         *
         * @returns A pair&lt;ComponentType&amp;,bool&gt;
         */
        [[nodiscard]] _STD pair<ComponentType *, bool> emplace(const entity_guid &guid_, ComponentType &&component_) {
            typename mapping_container::emplace_result_type er = _indirection.emplace(storage_guid_idx_pair{
                    guid_,
                    0
            });
            storage_guid_idx_pair &inp = er.first.value();

            /**
             * Check whether emplace at indirection failed
             */
            ComponentType *ptr;
            if (er.second == false) {
                /**
                 * If emplace failed, get reference of already existing element
                 */
                ptr = &_pages[unmask_page_index(inp.idx)].get_value(unmask_value_index(inp.idx));
            } else {
                /**
                 * If emplace succeeded, move construct element in pages compressed storage
                 */
                const index_type cidx = pages_insert_pair(guid_, _STD forward<ComponentType>(component_));

                /**
                 * Store cidx to indirection entry
                 */
                inp.idx = cidx;

                /**
                 * Get reference to existing element
                 */
                ptr = &_pages[unmask_page_index(cidx)].get_value(unmask_value_index(cidx));
            }

            return _STD make_pair(ptr, er.second);
        }

        /**
         * Inserts a component with the given unique identifier
         *
         * @author Julius
         * @date 30.10.2020
         *
         * @param  guid_ Unique identifier.
         *
         * @returns A pair&lt;ComponentType&amp;,bool&gt;
         */
        [[nodiscard]] _STD pair<ComponentType *, bool> insert(const entity_guid &guid_) {
            return emplace(guid_);
        }

        /**
         * Inserts a move constructed component with the given unique identifier
         *
         * @author Julius
         * @date 30.10.2020
         *
         * @param 		   guid_ Unique identifier.
         * @param [in,out] component_ The component.
         *
         * @returns A pair&lt;ComponentType&amp;,bool&gt;
         */
        [[nodiscard]] _STD pair<ComponentType *, bool> insert(const entity_guid &guid_, ComponentType &&component_) {
            return emplace(guid_, _STD forward<ComponentType>(component_));
        }

        /**
         * Inserts an or assign a guid component pair to storage
         *
         * @author Julius
         * @date 30.10.2020
         *
         * @param 		   guid_ Unique identifier.
         * @param [in,out] component_ The component.
         *
         * @returns A reference to a ComponentType.
         */
        ComponentType &insert_or_assign(const entity_guid &guid_, ComponentType &&component_) {
            /**
             * Don't insert_or_assign, cause we need old indirection idx from inserted element to override old value
             *	-> This also guarantees, that component constructed by entity_guid is reference stable
             */
            typename mapping_container::insert_or_assign_result_type ioar = _indirection.emplace(
                    storage_guid_idx_pair{guid_, 0});
            storage_guid_idx_pair &inp = ioar.first.value();

            /**
             * Check whether emplace failed
             */
            ComponentType *ptr;
            if (inp.second == false) {
                /**
                 * If emplace failed, replace existing value
                 */
                storage_page_type &page = _pages[unmask_page_index(inp.idx)];

                /**
                 * Replace existing value and key at storage page
                 */
                page.replace(unmask_value_index(inp.idx), guid_, _STD forward<ComponentType>(component_));

                /**
                 * Get reference to existing element
                 */
                ptr = &page.get_value(unmask_value_index(inp.idx));
            } else {
                /**
                 * If emplace succeeded, insert key component pair to storage page
                 */
                const index_type cidx = pages_insert_pair(guid_, _STD forward<ComponentType>(component_));

                /**
                 * Stored cidx to indirection entry
                 */
                inp.idx = cidx;

                /**
                 * Get reference to existing element
                 */
                ptr = &_pages[unmask_page_index(cidx)].get_value(unmask_value_index(cidx));
            }

            return *ptr;
        }

        /**
         * Gets a constant component reference using the given unique identifier
         *
         * @author Julius
         * @date 30.10.2020
         *
         * @exception _STD Thrown when a Standard error condition occurs.
         *
         * @param  guid_ The Unique identifier to get.
         *
         * @returns A reference to a const ComponentType.
         */
        const ComponentType &unsafe_get(const entity_guid &guid_) const {
            typename mapping_container::const_iterator cit = _indirection.find(storage_guid_idx_pair{guid_, 0});

            if (cit == _indirection.cend()) {
                throw _STD out_of_range(
                        "Can not get constant reference to component, while guid_ does not link to one.");
            }

            return _pages[unmask_page_index(cit.value().idx)].get_value(unmask_value_index(cit.value().idx));
        }

        /**
         * Gets a constant component reference using the given unique identifier
         *
         * @author Julius
         * @date 30.10.2020
         *
         * @exception _STD Thrown when a Standard error condition occurs.
         *
         * @param  guid_ The Unique identifier to get.
         * @param  hash_ The hash.
         *
         * @returns A reference to a const ComponentType.
         */
        const ComponentType &unsafe_get(const entity_guid &guid_, const hash_type hash_) const {
            typename mapping_container::const_iterator cit = _indirection.find(storage_guid_idx_pair{guid_, 0},
                                                                               hash_);

            if (cit == _indirection.cend()) {
                throw _STD out_of_range(
                        "Can not get constant reference to component, while guid_ does not link to one.");
            }

            return _pages[unmask_page_index(cit.value().idx)].get_value(unmask_value_index(cit.value().idx));
        }

        /**
         * Gets a component reference using the given unique identifier
         *
         * @author Julius
         * @date 30.10.2020
         *
         * @exception _STD Thrown when a Standard error condition occurs.
         *
         * @param  guid_ The Unique identifier to get.
         *
         * @returns A reference to a ComponentType.
         */
        ComponentType &unsafe_get(const entity_guid &guid_) {
            typename mapping_container::iterator cit = _indirection.find(storage_guid_idx_pair{guid_, 0});

            if (cit == _indirection.end()) {
                throw _STD out_of_range("Can not get reference to component, while guid_ does not link to one.");
            }

            return _pages[unmask_page_index(cit.value().idx)].get_value(unmask_value_index(cit.value().idx));
        }

        /**
         * Gets a component reference using the given unique identifier
         *
         * @author Julius
         * @date 30.10.2020
         *
         * @exception _STD Thrown when a Standard error condition occurs.
         *
         * @param  guid_ The Unique identifier to get.
         * @param  hash_ The hash.
         *
         * @returns A reference to a ComponentType.
         */
        ComponentType &unsafe_get(const entity_guid &guid_, const hash_type hash_) {
            typename mapping_container::iterator cit = _indirection.
                    find(storage_guid_idx_pair{guid_, 0}, hash_);

            if (cit == _indirection.end()) {
                throw _STD out_of_range("Can not get reference to component, while guid_ does not link to one.");
            }

            return _pages[unmask_page_index(cit.value().idx)].get_value(unmask_value_index(cit.value().idx));
        }

        /**
         * Gets a constant component pointer using the given unique identifier
         *
         * @author Julius
         * @date 02.11.2020
         *
         * @param  guid_ The Unique identifier to get.
         *
         * @returns Null if it fails, else a pointer to a const ComponentType.
         */
        const ComponentType *get(const entity_guid &guid_) const {
            typename mapping_container::const_iterator cit = _indirection.find(storage_guid_idx_pair{guid_, 0});

            if (cit == _indirection.cend()) {
                return nullptr;
            }

            return &_pages[unmask_page_index(cit.value().idx)].get_value(unmask_value_index(cit.value().idx));
        }

        /**
         * Gets a constant component pointer using the given unique identifier
         *
         * @author Julius
         * @date 02.11.2020
         *
         * @param  guid_ The Unique identifier to get.
         * @param  hash_ The hash.
         *
         * @returns Null if it fails, else a pointer to a const ComponentType.
         */
        const ComponentType *get(const entity_guid &guid_, const hash_type hash_) const {
            typename mapping_container::const_iterator cit = _indirection.find(storage_guid_idx_pair{guid_, 0},
                                                                               hash_);

            if (cit == _indirection.cend()) {
                return nullptr;
            }

            return &_pages[unmask_page_index(cit.value().idx)].get_value(unmask_value_index(cit.value().idx));
        }

        /**
         * Gets a component pointer using the given unique identifier
         *
         * @author Julius
         * @date 02.11.2020
         *
         * @param  guid_ The Unique identifier to get.
         *
         * @returns Null if it fails, else a pointer to a ComponentType.
         */
        ComponentType *get(const entity_guid &guid_) {
            typename mapping_container::iterator cit = _indirection.find(storage_guid_idx_pair{guid_, 0});

            if (cit == _indirection.end()) {
                return nullptr;
            }

            return &_pages[unmask_page_index(cit->idx)].get_value(unmask_value_index(cit->idx));
        }

        /**
         * Gets a component pointer using the given unique identifier
         *
         * @author Julius
         * @date 02.11.2020
         *
         * @param  guid_ The Unique identifier to get.
         * @param  hash_ The hash.
         *
         * @returns Null if it fails, else a pointer to a ComponentType.
         */
        ComponentType *get(const entity_guid &guid_, const hash_type hash_) {
            typename mapping_container::iterator cit = _indirection.
                    find(storage_guid_idx_pair{guid_, 0}, hash_);

            if (cit == _indirection.end()) {
                return nullptr;
            }

            return &_pages[unmask_page_index(cit->idx)].get_value(unmask_value_index(cit->idx));
        }

        /**
         * Erases component identified by given guid_
         *
         * @author Julius
         * @date 30.10.2020
         *
         * @exception _STD Thrown when a Standard error condition occurs.
         *
         * @param  guid_ Unique identifier.
         */
        void erase(const entity_guid &guid_) {
            typename mapping_container::const_iterator cit = _indirection.find(storage_guid_idx_pair{guid_, 0});
            // TODO: check whether we should just return without mutation
            // TODO: if immutable check, could optimize for concurrent locking, cause existing check is read only, and does not require to lock whole structure

            /**
             * If guid_ does not exist, catch error case
             */
            if (cit == _indirection.end()) {
                throw _STD out_of_range("Can not erase element from storage, while _guid does not link to one");
            }

            /**
             * Get mutation reference to storage page
             */
            storage_page_type &page = _pages[unmask_page_index(cit->idx)];

            /**
             * Erase value and key pair form storage page
             */
            page.erase(unmask_value_index(cit->idx));

            /**
             * Cleanup key storage
             */
            _indirection.erase(cit);
        }

        /**
         * Gets the begin
         *
         * @author Julius
         * @date 02.11.2020
         *
         * @returns An iterator.
         */
        [[nodiscard]] iterator begin() noexcept {
            return iterator{
                    _pages.begin()
            };
        }

        /**
         * Gets the end
         *
         * @author Julius
         * @date 02.11.2020
         *
         * @returns An iterator.
         */
        [[nodiscard]] iterator end() noexcept {
            auto pe = _pages.end();

            return iterator{
                    pe,
                    pe->end()
            };
        }

        /**
         * Gets the begin
         *
         * @author Julius
         * @date 02.11.2020
         *
         * @returns A const_iterator.
         */
        [[nodiscard]] const_iterator begin() const noexcept {
            return const_iterator{
                    _pages.cbegin()
            };
        }

        /**
         * Gets the end
         *
         * @author Julius
         * @date 02.11.2020
         *
         * @returns A const_iterator.
         */
        [[nodiscard]] const_iterator end() const noexcept {
            auto pe = _pages.cend();

            return const_iterator{
                    pe,
                    pe->end()
            };
        }

        /**
         * Gets the cbegin
         *
         * @author Julius
         * @date 02.11.2020
         *
         * @returns A const_iterator.
         */
        [[nodiscard]] const_iterator cbegin() const noexcept {
            return const_iterator{
                    _pages.cbegin()
            };
        }

        /**
         * Gets the cend
         *
         * @author Julius
         * @date 02.11.2020
         *
         * @returns A const_iterator.
         */
        [[nodiscard]] const_iterator cend() const noexcept {
            auto pe = _pages.cend();

            return const_iterator{
                    pe,
                    pe->end()
            };
        }

    private:
        vector<storage_page_type> _pages;

        /**
         * Unmask page index
         *
         * @author Julius
         * @date 30.10.2020
         *
         * @param  masked_ The masked.
         *
         * @returns A size_t.
         */
        FORCE_INLINE static page_index_type unmask_page_index(const uint64_t masked_) {
            return static_cast<page_index_type>((masked_ & index_page_mask) >> index_page_shift);
        }

        /**
         * Unmask value index
         *
         * @author Julius
         * @date 30.10.2020
         *
         * @param  masked_ The masked.
         *
         * @returns A size_t.
         */
        FORCE_INLINE static value_index_type unmask_value_index(const uint64_t masked_) {
            return static_cast<value_index_type>(masked_ & index_value_mask);
        }

        /**
         * Unmasks
         *
         * @author Julius
         * @date 30.10.2020
         *
         * @param 		   masked_ The masked.
         * @param [in,out] page_ The page.
         * @param [in,out] value_ The value.
         */
        FORCE_INLINE static void unmask(IN const uint64_t masked_, OUT uint64_t &page_, OUT uint64_t &value_) {
            page_ = (masked_ & index_page_mask) >> index_page_shift;
            value_ = (masked_ & index_value_mask);
        }

        /** The indirection */
        mapping_container _indirection;

    public:
        /**
         * Gets the composite indirection pair collection
         *
         * @author Julius
         * @date 30.10.2020
         *
         * @returns A list of.
         */
        const mapping_container &keys() const noexcept {
            return _indirection;
        }

    private:
        /**
         * Insert key value pair to storage pages
         *
         * @author Julius
         * @date 30.10.2020
         *
         * @param  guid_ Unique identifier.
         * @param  component_ The component.
         *
         * @returns An index_type.
         */
        [[nodiscard]] index_type pages_insert_pair(const entity_guid &guid_, const ComponentType &component_) {
            // TODO: optimize way to find storage page with empty sequence
            auto s = _STD find_if(_pages.begin(), _pages.end(), [](const storage_page_type &page_) {
                return page_.can_store();
            });

            /**
             * Check whether there is a page with empty space to store data
             */
            if (s == _pages.end()) {

                /**
                 * If no space found, create new page and try again
                 */
                _pages.push_back(storage_page_type{});

                /**
                 * Recursive try again
                 */
                return pages_insert_pair(guid_, component_);
            }

            /**
             * Get mutation reference to storage page
             */
            storage_page_type &page = *s;

            /**
             * Emplace data to page
             */
            index_type vidx;
            [[maybe_unused]] const bool success = page.emplace(guid_, component_, vidx);

            const index_type pidx = _STD distance(_pages.begin(), s);

            /**
             * Composite index_type
             */
            return (vidx | (pidx << index_page_shift));
        }

        /**
         * Insert key value pair to storage pages
         *
         * @author Julius
         * @date 30.10.2020
         *
         * @param 		   guid_ Unique identifier.
         * @param [in,out] component_ The component.
         *
         * @returns An index_type.
         */
        [[nodiscard]] index_type pages_insert_pair(const entity_guid &guid_, ComponentType &&component_) {
            // TODO: optimize way to find storage page with empty sequence
            auto s = _STD find_if(_pages.begin(), _pages.end(), [](const storage_page_type &page_) {
                return page_.can_store();
            });

            /**
             * Check whether there is a page with empty space to store data
             */
            if (s == _pages.end()) {

                /**
                 * If no space found, create new page and try again
                 */
                _pages.push_back(storage_page_type{});

                /**
                 * Recursive try again
                 */
                return pages_insert_pair(guid_, _STD forward<ComponentType>(component_));
            }

            /**
             * Get mutation reference to storage page
             */
            storage_page_type &page = *s;

            /**
             * Emplace data to page
             */
            value_index_type vidx;
            [[maybe_unused]] const bool success = page.emplace(guid_, _STD forward<ComponentType>(component_), vidx);

            const page_index_type pidx = _STD distance(_pages.begin(), s);

            /**
             * Composite index_type
             */
            return (index_type{vidx} | (index_type{pidx} << index_page_shift));
        }
    };
}
