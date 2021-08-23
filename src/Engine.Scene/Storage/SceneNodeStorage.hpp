#pragma once

#include <Engine.Common/Types.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Collection/HopScotch.hpp>
#include <Engine.Common/Collection/List.hpp>

namespace ember::engine::scene {

    /**
     * Construct inplace
     *
     * @tparam Ty Type of the ty.
     * @tparam Alloc_ Type of the allocate.
     * @param  dst_ Destination for the.
     * @param  allocator_ (Optional) The allocator.
     *
     * @returns A ptr&lt;Ty&gt;
     */
    template <typename Ty, typename Alloc_ = _STD allocator<Ty>>
    FORCEINLINE constexpr ptr<Ty> construct_inplace(ptr<void> dst_, Alloc_ allocator_ = Alloc_ {}) {
        using allocator_trait = _STD allocator_traits<Alloc_>;
        allocator_trait::construct(allocator_, static_cast<ptr<Ty>>(dst_));
        return static_cast<ptr<Ty>>(dst_);
    }

    /**
     * Copy Construct inplace
     *
     * @tparam Ty Type of the ty.
     * @tparam Alloc_ Type of the allocate.
     * @param  dst_ Destination for the.
     * @param  src_ Source for the.
     * @param  allocator_ (Optional) The allocator.
     *
     * @returns A ptr&lt;Ty&gt;
     */
    template <typename Ty, typename Alloc_ = _STD allocator<Ty>>
    FORCEINLINE constexpr ptr<Ty> construct_inplace(ptr<void> dst_, cref<Ty> src_, Alloc_ allocator_ = Alloc_ {}) {
        using allocator_trait = _STD allocator_traits<Alloc_>;
        allocator_trait::construct(allocator_, static_cast<ptr<Ty>>(dst_), _STD forward<cref<Ty>>(src_));
        return static_cast<ptr<Ty>>(dst_);
    }

    /**
     * Move Construct inplace
     *
     * @tparam Ty Type of the ty.
     * @tparam Alloc_ Type of the allocate.
     * @param  dst_ Destination for the.
     * @param  src_ Source for the.
     * @param  allocator_ (Optional) The allocator.
     *
     * @returns A ptr&lt;Ty&gt;
     */
    template <typename Ty, typename Alloc_ = _STD allocator<Ty>>
    FORCEINLINE constexpr ptr<Ty> construct_inplace(ptr<void> dst_, mref<Ty> src_, Alloc_ allocator_ = Alloc_ {}) {
        using allocator_trait = _STD allocator_traits<Alloc_>;
        allocator_trait::construct(allocator_, static_cast<ptr<Ty>>(dst_), _STD forward<Ty>(src_));
        return static_cast<ptr<Ty>>(dst_);
    }

    /**
     * Destruct inplace
     *
     * @tparam Ty Type of the ty.
     * @tparam Alloc_ Type of the allocate.
     * @param  dst_ Destination for the.
     * @param  allocator_ (Optional) The allocator.
     */
    template <typename Ty, typename Alloc_ = _STD allocator<Ty>>
    FORCEINLINE constexpr void destruct_inplace(
        ptr<Ty> dst_,
        Alloc_ allocator_ = Alloc_ {}
    ) noexcept(_STD is_nothrow_destructible_v<Ty>) {
        using allocator_trait = _STD allocator_traits<Alloc_>;
        allocator_trait::destroy(allocator_, dst_);
    }

    /**
     * Forward declaration for scene_node_storage
     *
     * @author Julius
     * @date 04.08.2021
     *
     * @tparam ValueType Type of the value type.
     * @tparam KeyType Type of the key type.
     */
    template <typename ValueType, typename KeyType>
    class scene_node_storage;

    /**
     * A scene node storage page. This class cannot be inherited..
     *
     * @author Julius
     * @date 05.08.2021
     *
     * @tparam ValueType Type of the value type.
     * @tparam KeyType Type of the key type.
     * @tparam IndexType Type of the index type.
     */
    template <typename ValueType, typename KeyType, typename IndexType = u64>
    class scene_node_storage_page final {

        friend class scene_node_storage<ValueType, KeyType>;

    public:
        using this_type = scene_node_storage_page<ValueType, KeyType, IndexType>;
        using reference_type = ref<this_type>;
        using const_reference_type = cref<this_type>;
        using pointer_type = ptr<this_type>;

        using index_type = IndexType;

        using key_type = KeyType;
        using key_allocator_type = _STD allocator<key_type>;
        using key_allocator_traits = _STD allocator_traits<key_allocator_type>;

        using value_type = ValueType;
        using value_allocator_type = _STD allocator<value_type>;
        using value_allocator_traits = _STD allocator_traits<value_allocator_type>;

    public:
        inline constexpr static size_t key_size = sizeof(key_type);
        inline constexpr static size_t value_size = sizeof(value_type);

        // inline constexpr static size_t page_size = 32786;// 32kB Memory Blocks
        inline constexpr static size_t page_size = 1 << 20;// 1MB Memory Blocks
        inline constexpr static size_t per_page = page_size / (key_size + value_size);

    protected:
        /**
         * Default constructor
         *
         * @author Julius
         * @date 04.08.2021
         */
        explicit scene_node_storage_page() noexcept :
            _mem(allocate_safe()),
            _keys(static_cast<ptr<key_type>>(static_cast<ptr<void>>(static_cast<ptr<value_type>>(_mem) + per_page))),
            _values(static_cast<ptr<value_type>>(_mem)),
            _seq(vector<sequence> { sequence { 0, per_page } }) { }

    public:
        /**
         * Copy Constructor
         *
         * @author Julius
         * @date 04.08.2021
         */
        scene_node_storage_page(const_reference_type) = delete;

        /**
         * Move Constructor
         *
         * @author Julius
         * @date 04.08.2021
         *
         * @param  other_ The other.
         */
        scene_node_storage_page(mref<this_type> other_) noexcept :
            _mem(_STD exchange(other_._mem, nullptr)),
            _keys(_STD exchange(other_._keys, nullptr)),
            _values(_STD exchange(other_._values, nullptr)),
            _seq(_STD move(other_._seq)) { }

        /**
         * Destructor
         *
         * @author Julius
         * @date 04.08.2021
         */
        ~scene_node_storage_page() noexcept {
            tidy();
        }

    private:
        /**
         * The internal managed and containing memory block
         */
        void* _mem;

        ptr<key_type> _keys;/* := _mem[per_page * sizeof(value_type)] */
        ptr<value_type> _values;/* := _mem[0] */

    private:
        /**
         * Allocate memory block and guarantees empty data for key space
         *
         * @author Julius
         * @date 04.08.2021
         *
         * @returns A ptr&lt;void&gt;
         */
        static constexpr ptr<void> allocate_safe() noexcept {
            void* page = malloc(this_type::page_size);

            /**
             *
             */
            constexpr auto key_length = key_size * per_page;
            ptr<key_type> key_ptr = static_cast<ptr<key_type>>(
                static_cast<ptr<void>>(
                    static_cast<ptr<value_type>>(page) + per_page
                )
            );

            /**
             * Cleanup memory block with empty data
             */
            memset(key_ptr, 0, key_length);

            /**
             *
             */
            return page;
        }

        /**
         * Tidies this 
         *
         * @author Julius
         * @date 04.08.2021
         */
        void tidy() noexcept {
            if (_mem != nullptr) {
                // TODO:
                // destroy_values(ptr<value_type> from_, ptr<value_type> to_);
                // destroy_keys(ptr<key_type> from_, ptr<key_type> to_);
                for (auto entry : *this) {

                    ptr<key_type> key = const_cast<ptr<key_type>>(entry.first);
                    ptr<value_type> value = const_cast<ptr<value_type>>(entry.second);

                    if (*key) {
                        destruct_inplace<key_type, key_allocator_type>(key);
                        destruct_inplace<value_type, value_allocator_type>(value);
                    }
                }

                free(_mem);
                _mem = nullptr;
            }
        }

    private:
        struct sequence {
            using value_type = sequence;
            using reference_type = ref<value_type>;
            using const_reference_type = cref<value_type>;

            index_type first;
            index_type last;

            /**
             * Merges the given index into this sequence
             *
             * @author Julius
             * @date 04.08.2021
             *
             * @param  index_ Zero-based index of the.
             *
             * @returns A reference_type.
             */
            reference_type merge(cref<index_type> index_) noexcept {
                return index_ < first ? --first : ++last, *this;
            }

            /**
             * Merges the given and this sequence into one
             *
             * @author Julius
             * @date 04.08.2021
             *
             * @param  other_ The other.
             *
             * @returns A reference_type.
             */
            reference_type merge(const_reference_type other_) noexcept {
                first = MIN(first, other_.first);
                last = MAX(last, other_.last);
                return *this;
            }

            /**
             * Check whether given index can be merged into sequence
             *
             * @author Julius
             * @date 04.08.2021
             *
             * @param  index_ Zero-based index of the.
             *
             * @returns True if it succeeds, false if it fails.
             */
            [[nodiscard]] bool mergable(cref<index_type> index_) const noexcept {
                return (last + 1u == index_) || (first - 1u == index_);
            }

            /**
             * Check whether given sequence can be merged into sequence
             *
             * @author Julius
             * @date 04.08.2021
             *
             * @param  other_ The other.
             *
             * @returns True if it succeeds, false if it fails.
             */
            [[nodiscard]] bool mergable(const_reference_type other_) const noexcept {
                return (last + 1u <= other_.first) || (first - 1u >= other_.last);
            }

            /**
             * Splits this sequence into two sub-sequence by excluding given element
             *
             * @author Julius
             * @date 04.08.2021
             *
             * @param  exclude_ The exclude.
             *
             * @returns A pair&lt;value_type,value_type&gt;
             */
            [[nodiscard]] _STD pair<value_type, value_type> split(cref<index_type> exclude_) const noexcept {
                return {
                    { first, exclude_ - 1u },
                    { exclude_ + 1u, last }
                };
            }

            /**
             * Check whether this described a valid forward sequence
             *
             * @author Julius
             * @date 04.08.2021
             *
             * @returns True if it succeeds, false if it fails.
             */
            [[nodiscard]] bool valid() const noexcept {
                return first <= last;
            }

            /**
             * Get the amount of elements described by this sequence
             *
             * @author Julius
             * @date 04.08.2021
             *
             * @returns An index_type.
             */
            [[nodiscard]] index_type length() const noexcept {
                return (last + 1u) - first;
            }

            /**
             * Query if this contains the given index_
             *
             * @author Julius
             * @date 04.08.2021
             *
             * @param  index_ The cref&lt;index_type&gt; to test for containment.
             *
             * @returns True if the object is in this collection, false if not.
             */
            [[nodiscard]] bool contains(cref<index_type> index_) const noexcept {
                return index_ >= first && index_ <= last;
            }

            /**
             * Increment the first index
             *
             * @author Julius
             * @date 04.08.2021
             *
             * @returns The result of the operation.
             */
            reference_type operator++() noexcept {
                return ++first, *this;
            }

            /**
             * Decrement the first index
             *
             * @author Julius
             * @date 04.08.2021
             *
             * @returns The result of the operation.
             */
            reference_type operator--() noexcept {
                return --first, *this;
            }

            /**
             * Increment the last index
             *
             * @author Julius
             * @date 04.08.2021
             *
             * @returns The result of the operation.
             */
            reference_type operator++(int) noexcept {
                return ++last, *this;
            }

            /**
             * Decrement the last index
             *
             * @author Julius
             * @date 04.08.2021
             *
             * @returns The result of the operation.
             */
            reference_type operator--(int) noexcept {
                return --last, *this;
            }

            /**
             * Removes and returns the first element
             *
             * @author Julius
             * @date 04.08.2021
             *
             * @returns An index_type.
             */
            [[nodiscard]] index_type pop_front() noexcept {
                return first++;
            }

            /**
             * Removes and return the last element
             *
             * @author Julius
             * @date 04.08.2021
             *
             * @returns An index_type.
             */
            [[nodiscard]] index_type pop_back() noexcept {
                return last--;
            }
        };

        /**
         * A sequence range comparator.
         *
         * @author Julius
         * @date 04.08.2021
         */
        struct sequence_range_comparator {
            [[nodiscard]] bool operator()(
                typename sequence::const_reference_type left_,
                typename sequence::const_reference_type right_
            ) const noexcept {
                return left_.length() > right_.length();
            }
        };

        /**
         * A sequence start comparator.
         *
         * @author Julius
         * @date 05.08.2021
         */
        struct sequence_start_comparator {
            [[nodiscard]] bool operator()(
                typename sequence::const_reference_type left_,
                typename sequence::const_reference_type right_
            ) const noexcept {
                return left_.first < right_.first;
            }
        };

        /** Type of the sequence comparator */
        using sequence_comparator_type = sequence_start_comparator;

    private:
        /**
         * Empty sequences within page
         */
        vector<sequence> _seq;

    public:
        /**
         * Determine if this can store another element
         *
         * @author Julius
         * @date 04.08.2021
         *
         * @returns True if we can store, false if not.
         */
        [[nodiscard]] bool can_store() const noexcept {
            return !_seq.empty();
        }

        /**
         * Gets a key-value pair using the given index
         *
         * @author Julius
         * @date 04.08.2021
         *
         * @param  idx_ The Index to get.
         *
         * @returns A pair&lt;cref&lt;key_type&gt;,cref&lt;value_type&gt;&gt;
         */
        [[nodiscard]] _STD pair<cref<key_type>, cref<value_type>> get(cref<index_type> idx_) const {
            return {
                _keys[idx_],
                _values[idx_]
            };
        }

        /**
         * Gets a key-value pair using the given index
         *
         * @author Julius
         * @date 04.08.2021
         *
         * @param  idx_ The Index to get.
         *
         * @returns A pair&lt;cref&lt;key_type&gt;,ref&lt;value_type&gt;&gt;
         */
        [[nodiscard]] _STD pair<cref<key_type>, ref<value_type>> get(cref<index_type> idx_) {
            return {
                _keys[idx_],
                _values[idx_]
            };
        }

        /**
         * Get a pointer to the addressed key
         *
         * @author Julius
         * @date 16.08.2021
         *
         * @param  pos_ The position.
         *
         * @returns A ptr&lt;key_type&gt;
         */
        [[nodiscard]] ptr<key_type> key_at(cref<index_type> pos_) const noexcept {
            return &_keys[pos_];
        }

        /**
         * Get a pointer to the addressed key
         *
         * @author Julius
         * @date 16.08.2021
         *
         * @param  pos_ The position.
         *
         * @returns A ptr&lt;key_type&gt;
         */
        [[nodiscard]] ptr<key_type> key_at(cref<index_type> pos_) noexcept {
            return &_keys[pos_];
        }

        /**
         * Get a pointer to the addressed value
         *
         * @author Julius
         * @date 16.08.2021
         *
         * @param  pos_ The position.
         *
         * @returns A ptr&lt;value_type&gt;
         */
        [[nodiscard]] ptr<value_type> value_at(cref<index_type> pos_) const noexcept {
            return &_values[pos_];
        }

        /**
         * Get a pointer to the addressed value
         *
         * @author Julius
         * @date 16.08.2021
         *
         * @param  pos_ The position.
         *
         * @returns A ptr&lt;value_type&gt;
         */
        [[nodiscard]] ptr<value_type> value_at(cref<index_type> pos_) noexcept {
            return &_values[pos_];
        }

    private:
        /**
         * Replaces an key-value pair at the given index
         *
         * @author Julius
         * @date 04.08.2021
         *
         * @param  idx_ Zero-based index of the.
         * @param  key_ The key.
         * @param  value_ The value.
         */
        void replace(
            cref<index_type> idx_,
            cref<key_type> key_,
            IN mref<value_type> value_
        ) noexcept (_STD is_nothrow_move_assignable_v<value_type>) {

            if constexpr (_STD is_nothrow_move_assignable_v<key_type>) {
                _keys[idx_] = _STD move(key_);
            } else {
                destruct_inplace<key_type, key_allocator_type>(&_keys[idx_]);
                construct_inplace<key_type, key_allocator_type>(&_keys[idx_], _STD move(key_));
            }

            if constexpr (_STD is_nothrow_move_assignable_v<value_type>) {
                _values[idx_] = _STD move(value_);
            } else {
                destruct_inplace<value_type, value_allocator_type>(&_values[idx_]);
                construct_inplace<value_type, value_allocator_type>(&_values[idx_], _STD forward<value_type>(value_));
            }
        }

        /**
         * Pops a slot from sequences if available, otherwise return invalid index (~0)
         *
         * @author Julius
         * @date 05.08.2021
         *
         * @returns An index_type.
         */
        [[nodiscard]] index_type pop_slot() noexcept {
            /**
             * Check whether sequence is available
             */
            if (_seq.empty()) {
                return static_cast<index_type>(~0);
            }

            /**
             * Get upper bound sequence
             */
            auto& bit = _seq.back();

            /**
             * Get first element from sequence and pop it
             */
            const auto slot = bit.pop_front();

            /**
             * If sequence got invalid, erase from collection
             */
            if (!bit.valid()) {
                _seq.pop_back();
            }

            return slot;
        }

        /**
         * Pops the slot from sequences described by idx_
         *
         * @author Julius
         * @date 04.08.2021
         *
         * @param  idx_ Zero-based index of the.
         */
        void pop_slot(cref<index_type> idx_) noexcept {
            /**
             * Find sequence containing idx_
             */
            auto sit = _STD find_if(_seq.rbegin(), _seq.rend(),
                [&idx_](typename sequence::const_reference_type sequence_) {
                    return sequence_.contains(idx_);
                });

            /**
             * Check whether sequence with idx_ was found
             */
            if (sit == _seq.rend()) {
                return;
            }

            /**
             *
             */
            if (sit->first == idx_) {
                /**
                 * If idx_ is first element, pre-decrement
                 */
                --(*sit);

                /**
                 * If sequence got invalid, erase from collection
                 */
                if (!sit->valid()) {
                    _seq.erase(sit);
                }
            } else if (sit->last == idx_) {
                /**
                 * If idx_ is last element, post-decrement
                 */
                (*sit)--;

                /**
                 * If sequence got invalid, erase from collection
                 */
                if (!sit->valid()) {
                    _seq.erase(sit);
                }
            } else {
                /**
                 * If idx_ is not boundary of sequence, exclude idx_
                 */
                auto sub = sit->split(idx_);

                /**
                 * Erase old sequence from sequence collection
                 */
                _seq.erase(sit);

                /**
                 * Push new sub-sequences to collection
                 */
                _seq.push_back(_STD move(sub.first));
                _seq.push_back(_STD move(sub.last));

                _STD sort(_seq.beg(), _seq.end(), sequence_comparator_type {});
            }
        }

        /**
         * Places an key-value pair at the given index
         *
         * @author Julius
         * @date 04.08.2021
         *
         * @param  idx_ Zero-based index of the.
         * @param  key_ The key.
         * @param  value_ The value.
         */
        void place(
            cref<index_type> idx_,
            cref<key_type> key_,
            IN mref<value_type> value_
        ) noexcept(_STD is_nothrow_move_constructible_v<value_type>) {
            construct_inplace<key_type, key_allocator_type>(&_keys[idx_], key_);
            construct_inplace<value_type, value_allocator_type>(&_values[idx_], _STD forward<value_type>(value_));
        }

    public:
        /**
         * Inserts a key-value pair at given index ( replaces existing )
         *
         * @author Julius
         * @date 04.08.2021
         *
         * @param  idx_ Zero-based index of the.
         * @param  key_ The key.
         * @param  value_ The value.
         */
        void insert(
            cref<index_type> idx_,
            cref<key_type> key_,
            IN mref<value_type> value_
        ) noexcept(_STD is_nothrow_move_assignable_v<value_type>) {
            /**
             * Check whether key exists
             */
            if (_keys[idx_]) {
                /**
                 * If index in range, override stored element
                 */
                replace(idx_, key_, _STD forward<value_type>(value_));
            } else {
                /**
                 * Reserve Slot from sequence
                 */
                pop_slot(idx_);

                /**
                 * Place the new key-value pair to local storage
                 */
                place(idx_, key_, _STD forward<value_type>(value_));
            }
        }

        /**
         * Inserts a key-value pair and references it's index
         *
         * @author Julius
         * @date 16.08.2021
         *
         * @param  key_ The key.
         * @param  value_ The value.
         * @param  idx_ Zero-based index of the.
         *
         * @returns True if it succeeds, false if it fails.
         */
        bool insert(
            IN cref<key_type> key_,
            IN mref<value_type> value_,
            OUT ref<index_type> idx_
        ) noexcept (_STD is_nothrow_move_assignable_v<value_type>) {
            /**
             * Get empty slot
             */
            idx_ = pop_slot();

            /**
             * Guard from invalid slots
             */
            if (idx_ > per_page) {
                return false;
            }

            /**
             * Place key and value to local storage
             */
            place(idx_, key_, _STD forward<value_type>(value_));

            /**
             * Return succeeded operation
             */
            return true;
        }

    private:
        /**
         * Restore the slot to sequences described by idx_
         *
         * @author Julius
         * @date 04.08.2021
         *
         * @param  idx_ Zero-based index of the.
         */
        void restore_slot(cref<index_type> idx_) noexcept {
            /**
             * Find upper bound sequence where index can be merged
             */
            auto uit = _STD find_if(_seq.rbegin(), _seq.rend(),
                [&idx_](typename sequence::const_reference_type sequence_) {
                    return sequence_.mergable(idx_);
                });

            /**
             * If no mergable sequence, create a new one
             */
            if (uit == _seq.end()) {
                uit = _seq.emplace(_seq.end(), { idx_, idx_ });
                _STD sort(_seq.begin(), _seq.end(), sequence_comparator_type {});
                return;
            }

            /**
             * Find lower bound sequence where index can be merged
             */
            auto lit = _STD find_if(_seq.begin(), _seq.end(),
                [&idx_](typename sequence::const_reference_type sequence_) {
                    return sequence_.mergable(idx_);
                });

            /**
             * If only one sequence can merge idx_
             */
            if (uit == lit) {
                /**
                 * Merge index into only available sequence
                 */
                uit->merge(idx_);
            } else {
                /**
                 * Merge index into upper bound sequence
                 */
                uit->merge(idx_);

                /**
                 * Copy upper bound sequence temporary
                 */
                auto uitv = *uit;

                /**
                 * Merge lower bound sequence with upper bound
                 */
                uitv.merge(*lit);

                /**
                 * Erase the forward and reverse iterator while there references are still stable
                 */
                _seq.erase(uit);
                _seq.erase(lit);

                /**
                 * Push new combined sequence to sequence collection
                 */
                _seq.push_back(_STD move(uitv));
            }

            _STD sort(_seq.begin(), _seq.end(), sequence_comparator_type {});
        }

    public:
        /**
         * Erases the key-value pair at the given index
         *
         * @author Julius
         * @date 05.08.2021
         *
         * @param  idx_ Zero-based index of the.
         */
        void erase(cref<index_type> idx_) noexcept(_STD is_nothrow_destructible_v<value_type>) {
            /**
             * Destroy key-value pair
             */
            destruct_inplace<key_type, key_allocator_type>(&_keys[idx_]);
            destruct_inplace<value_type, value_allocator_type>(&_values[idx_]);

            /**
             * Restore used slot to sequences
             */
            restore_slot(idx_);
        }

    private:
        template <typename ValueType_, bool Const_>
        class page_seg_iterator_base {
        public:
            using iterator_category = _STD contiguous_iterator_tag;

            using this_type = page_seg_iterator_base<ValueType_, Const_>;
            using reference_type = ref<this_type>;
            using const_reference_type = cref<this_type>;

            using value_type = _STD conditional_t<Const_, ValueType_, const ValueType_>;
            using value_ptr_type = ptr<value_type>;

            using index_type = IndexType;

        private:
            value_ptr_type _pos;

        public:
            page_seg_iterator_base() noexcept :
                _pos(nullptr) {}

            page_seg_iterator_base(const_reference_type other_) noexcept :
                _pos(other_._pos) {}

            page_seg_iterator_base(mref<this_type> other_) noexcept :
                _pos(_STD exchange(other_._pos, nullptr)) {}

            page_seg_iterator_base(value_ptr_type pos_) noexcept :
                _pos(pos_) {}

            ~page_seg_iterator_base() noexcept = default;

        public:
            reference_type operator=(const_reference_type other_) noexcept {
                return _pos = other_._pos, *this;
            }

            reference_type operator=(mref<this_type> other_) noexcept {
                if (this != &other_) {
                    _pos = _STD exchange(other_._pos, nullptr);
                }

                return *this;
            }

        public:
            reference_type operator++() noexcept {
                return ++_pos, *this;
            }

            reference_type operator++(int) noexcept {
                auto cpy { *this };
                ++(*this);
                return cpy;
            }

            reference_type operator--() noexcept {
                return --_pos, *this;
            }

            reference_type operator--(int) noexcept {
                auto cpy { *this };
                --(*this);
                return cpy;
            }

        public:
            [[nodiscard]] this_type operator+(cref<index_type> range_) const noexcept {
                return this_type { *this } += range_;
            }

            [[nodiscard]] this_type operator-(cref<index_type> range_) const noexcept {
                return this_type { *this } -= range_;
            }

            reference_type operator+=(cref<index_type> range_) noexcept {
                return _pos += range_, *this;
            }

            reference_type operator-=(cref<index_type> range_) noexcept {
                return _pos -= range_, *this;
            }

        public:
            [[nodiscard]] bool operator==(const_reference_type other_) const noexcept {
                return _pos == other_._pos;
            }

            [[nodiscard]] bool operator!=(const_reference_type other_) const noexcept {
                return _pos != other_._pos;
            }

        public:
            [[nodiscard]] bool operator<(const_reference_type other_) const noexcept {
                return _pos < other_._pos;
            }

            [[nodiscard]] bool operator>(const_reference_type other_) const noexcept {
                return _pos > other_._pos;
            }

            [[nodiscard]] bool operator<=(const_reference_type other_) const noexcept {
                return _pos <= other_._pos;
            }

            [[nodiscard]] bool operator>=(const_reference_type other_) const noexcept {
                return _pos >= other_._pos;
            }

        public:
            [[nodiscard]] ref<value_type> operator[](cref<size_t> range_) const noexcept {
                return const_cast<value_type>(_pos[range_]);
            }

        public:
            cref<value_type> operator*() const noexcept {
                return *_pos;
            }

            ref<value_type> operator*() noexcept {
                return *_pos;
            }

            const value_ptr_type operator->() const noexcept {
                return _pos;
            }

            value_ptr_type operator->() noexcept {
                return _pos;
            }
        };

    private:
        /**
         * A page key iterator. This class cannot be inherited..
         *
         * @author Julius
         * @date 05.08.2021
         *
         * @tparam Const_ Type of the constant.
         */
        template <bool Const_>
        class page_key_iterator final :
            public page_seg_iterator_base<KeyType, Const_> { };

    public:
        using key_iterator = page_key_iterator<false>;
        using const_key_iterator = page_key_iterator<true>;

    private:
        /**
         * A page value iterator. This class cannot be inherited..
         *
         * @author Julius
         * @date 05.08.2021
         *
         * @tparam Const_ Type of the constant.
         */
        template <bool Const_>
        class page_value_iterator final :
            public page_seg_iterator_base<ValueType, Const_> { };

    public:
        using value_iterator = page_value_iterator<false>;
        using const_value_iterator = page_value_iterator<true>;

    private:
        /**
         * A page key value iterator. This class cannot be inherited..
         *
         * @author Julius
         * @date 05.08.2021
         *
         * @tparam Const_ Type of the constant.
         */
        template <bool Const_>
        class page_key_value_iterator final {
        public:
            using iterator_category = _STD contiguous_iterator_tag;

            using this_type = page_key_value_iterator<Const_>;
            using reference_type = ref<this_type>;
            using const_reference_type = cref<this_type>;

            using key_type = _STD conditional_t<Const_, KeyType, const KeyType>;
            using key_ptr_type = ptr<key_type>;

            using value_type = _STD conditional_t<Const_, ValueType, const ValueType>;
            using value_ptr_type = ptr<value_type>;

            using key_value_type = _STD pair<key_ptr_type, value_ptr_type>;
            using const_key_value_type = _STD pair<const key_ptr_type, const value_ptr_type>;

            using index_type = IndexType;

        private:
            key_ptr_type _key_pos;
            value_ptr_type _value_pos;

        public:
            page_key_value_iterator() noexcept :
                _key_pos(nullptr),
                _value_pos(nullptr) {}

            page_key_value_iterator(const_reference_type other_) noexcept :
                _key_pos(other_._key_pos),
                _value_pos(other_._value_pos) {}

            page_key_value_iterator(mref<this_type> other_) noexcept :
                _key_pos(_STD exchange(other_._key_pos, nullptr)),
                _value_pos(_STD exchange(other_._value_pos, nullptr)) {}

            page_key_value_iterator(key_ptr_type key_pos_, value_ptr_type value_pos_) noexcept :
                _key_pos(key_pos_),
                _value_pos(value_pos_) {}

            ~page_key_value_iterator() noexcept = default;

        public:
            reference_type operator=(const_reference_type other_) noexcept {
                return _key_pos = other_._key_pos, _value_pos = other_._value_pos, *this;
            }

            reference_type operator=(mref<this_type> other_) noexcept {
                if (this != &other_) {
                    _key_pos = _STD exchange(other_._key_pos, nullptr);
                    _value_pos = _STD exchange(other_._value_pos, nullptr);
                }

                return *this;
            }

        public:
            reference_type operator++() noexcept {
                return ++_key_pos, ++_value_pos, *this;
            }

            reference_type operator++(int) noexcept {
                auto cpy { *this };
                ++(*this);
                return cpy;
            }

            reference_type operator--() noexcept {
                return --_key_pos, --_value_pos, *this;
            }

            reference_type operator--(int) noexcept {
                auto cpy { *this };
                --(*this);
                return cpy;
            }

        public:
            [[nodiscard]] this_type operator+(cref<index_type> range_) const noexcept {
                return this_type { *this } += range_;
            }

            [[nodiscard]] this_type operator-(cref<index_type> range_) const noexcept {
                return this_type { *this } -= range_;
            }

            reference_type operator+=(cref<index_type> range_) noexcept {
                return _key_pos += range_, _value_pos += range_, *this;
            }

            reference_type operator-=(cref<index_type> range_) noexcept {
                return _key_pos -= range_, _value_pos -= range_, *this;
            }

        public:
            [[nodiscard]] bool operator==(const_reference_type other_) const noexcept {
                #ifdef _DEBUG
                return _key_pos == other_._key_pos && _value_pos == other_._value_pos;
                #else
                return _value_pos == other_._value_pos;
                #endif
            }

            [[nodiscard]] bool operator!=(const_reference_type other_) const noexcept {
                #ifdef _DEBUG
                return _key_pos != other_._key_pos || _value_pos != other_._value_pos;
                #else
                return _value_pos != other_._value_pos;
                #endif
            }

        public:
            [[nodiscard]] bool operator<(const_reference_type other_) const noexcept {
                #ifdef _DEBUG
                return _key_pos < other_._key_pos && _value_pos < other_._value_pos;
                #else
                return _value_pos < other_._value_pos;
                #endif
            }

            [[nodiscard]] bool operator>(const_reference_type other_) const noexcept {
                #ifdef _DEBUG
                return _key_pos > other_._key_pos && _value_pos > other_._value_pos;
                #else
                return _value_pos > other_._value_pos;
                #endif
            }

            [[nodiscard]] bool operator<=(const_reference_type other_) const noexcept {
                #ifdef _DEBUG
                return _key_pos <= other_._key_pos && _value_pos <= other_._value_pos;
                #else
                return _value_pos <= other_._value_pos;
                #endif
            }

            [[nodiscard]] bool operator>=(const_reference_type other_) const noexcept {
                #ifdef _DEBUG
                return _key_pos >= other_._key_pos && _value_pos >= other_._value_pos;
                #else
                return _value_pos >= other_._value_pos;
                #endif
            }

        public:
            [[nodiscard]] key_value_type operator[](cref<size_t> range_) const noexcept {
                return key_value_type {
                    const_cast<key_type>(_key_pos[range_]),
                    const_cast<value_type>(_value_pos[range_])
                };
            }

        public:
            [[nodiscard]] const_key_value_type operator*() const noexcept {
                return const_key_value_type {
                    _key_pos,
                    _value_pos
                };
            }

            [[nodiscard]] key_value_type operator*() noexcept {
                return key_value_type {
                    _key_pos,
                    _value_pos
                };
            }

            [[nodiscard]] const_key_value_type operator->() const noexcept {
                return const_key_value_type {
                    _key_pos,
                    _value_pos
                };
            }

            [[nodiscard]] key_value_type operator->() noexcept {
                return key_value_type {
                    _key_pos,
                    _value_pos
                };
            }

        public:
            [[nodiscard]] _STD size_t diff(const_reference_type other_) const noexcept {
                return MAX(_key_pos, other_._key_pos) - MIN(_key_pos, other_._key_pos);
            }

        public:
            [[nodiscard]] cref<key_type> get_key() const noexcept {
                return *_key_pos;
            }

            [[nodiscard]] ref<key_type> get_key() noexcept {
                return *_key_pos;
            }

            [[nodiscard]] page_value_iterator<Const_> get_key_iterator() const noexcept {
                return page_value_iterator<Const_> { const_cast<key_ptr_type>(_key_pos) };
            }

            [[nodiscard]] page_key_iterator<Const_> get_key_iterator() noexcept {
                return page_key_iterator<Const_> { _key_pos };
            }

            [[nodiscard]] cref<value_type> get_value() const noexcept {
                return *_value_pos;
            }

            [[nodiscard]] ref<value_type> get_value() noexcept {
                return *_value_pos;
            }

            [[nodiscard]] page_value_iterator<Const_> get_value_iterator() const noexcept {
                return page_value_iterator<Const_> { const_cast<value_ptr_type>(_value_pos) };
            }

            [[nodiscard]] page_value_iterator<Const_> get_value_iterator() noexcept {
                return page_value_iterator<Const_> { _value_pos };
            }
        };

    public:
        using iterator = page_key_value_iterator<false>;
        using const_iterator = page_key_value_iterator<true>;

        /**
         * Gets the begin
         *
         * @author Julius
         * @date 05.08.2021
         *
         * @returns An iterator.
         */
        [[nodiscard]] iterator begin() noexcept {
            return iterator {
                _keys,
                _values
            };
        }

        /**
         * Gets the end
         *
         * @author Julius
         * @date 05.08.2021
         *
         * @returns An iterator.
         */
        [[nodiscard]] iterator end() noexcept {
            return iterator {
                _keys,
                _values
            } += per_page;
        }

        /**
         * Gets the begin
         *
         * @author Julius
         * @date 05.08.2021
         *
         * @returns A const_iterator.
         */
        [[nodiscard]] const_iterator begin() const noexcept {
            return const_iterator {
                _keys,
                _values
            };
        }

        /**
         * Gets the end
         *
         * @author Julius
         * @date 05.08.2021
         *
         * @returns A const_iterator.
         */
        [[nodiscard]] const_iterator end() const noexcept {
            return const_iterator {
                _keys,
                _values
            } += per_page;
        }

        /**
         * Gets the begin
         *
         * @author Julius
         * @date 05.08.2021
         *
         * @returns A const_iterator.
         */
        [[nodiscard]] const_iterator cbegin() noexcept {
            return const_iterator {
                _keys,
                _values
            };
        }

        /**
         * Gets the end
         *
         * @author Julius
         * @date 05.08.2021
         *
         * @returns A const_iterator.
         */
        [[nodiscard]] const_iterator cend() noexcept {
            return const_iterator {
                _keys,
                _values
            } += per_page;
        }

        /**
         * Gets the begin
         *
         * @author Julius
         * @date 05.08.2021
         *
         * @returns A const_iterator.
         */
        [[nodiscard]] const_iterator cbegin() const noexcept {
            return const_iterator {
                _keys,
                _values
            };
        }

        /**
         * Gets the end
         *
         * @author Julius
         * @date 05.08.2021
         *
         * @returns A const_iterator.
         */
        [[nodiscard]] const_iterator cend() const noexcept {
            return const_iterator {
                _keys,
                _values
            } += per_page;
        }
    };

    template <typename ValueType, typename KeyType>
    class scene_node_storage {
    public:
        using this_type = scene_node_storage<ValueType, KeyType>;
        using reference_type = ref<this_type>;
        using const_reference_type = cref<this_type>;

        inline constexpr static u64 index_page_mask = 0xFFFFFFFF00000000ui64;
        inline constexpr static u64 index_value_mask = 0x00000000FFFFFFFFui64;
        inline constexpr static u8 index_page_shift = 32ui8;

        using page_index_type = u32;
        using value_index_type = u32;
        using index_type = u64;

        using storage_page_type = scene_node_storage_page<ValueType, KeyType, value_index_type>;

    private:
        /**
         * A storage unique key index pair.
         *
         * @author Julius
         * @date 04.08.2021
         */
        struct storage_key_idx_pair {
            KeyType key;
            index_type idx;
        };

        /**
         * A unique key index hasher.
         *
         * @author Julius
         * @date 04.08.2021
         */
        struct key_idx_hasher :
            private _STD hash<KeyType> {
            /**
             * Forward hashing call to KeyType specific hash function
             *
             * @author Julius
             * @date 04.08.2021
             *
             * @param  value_ The value.
             *
             * @returns The result of the operation.
             */
            [[nodiscard]] _STD size_t operator()(cref<storage_key_idx_pair> value_) const {
                return static_cast<cref<_STD hash<KeyType>>>(*this)(value_.key);
            }
        };

        /**
         * A unique key index equal comparator.
         *
         * @author Julius
         * @date 04.08.2021
         */
        struct key_idx_equal :
            private _STD equal_to<KeyType> {

            /**
             * Forward equal to call to KeyType specific equality comparison
             *
             * @author Julius
             * @date 16.08.2021
             *
             * @param  left_ The left.
             * @param  right_ The right.
             *
             * @returns The result of the operation.
             */
            [[nodiscard]] bool operator()(
                cref<storage_key_idx_pair> left_,
                cref<storage_key_idx_pair> right_
            ) const noexcept {
                return static_cast<cref<_STD equal_to<KeyType>>>(*this)(left_.key, right_.key);
            }
        };

        /**
         * A unique key index less comparator.
         *
         * @author Julius
         * @date 04.08.2021
         */
        struct key_idx_less :
            private _STD less<KeyType> {
            /**
             * Forward less call to KeyType specific less comparison
             *
             * @author Julius
             * @date 16.08.2021
             *
             * @param  value_ The value.
             *
             * @returns The result of the operation.
             */
            [[nodiscard]] bool operator()(cref<storage_key_idx_pair> value_) const noexcept {
                return static_cast<cref<_STD less<KeyType>>>(*this)(value_.key);
            }
        };

    public:
        using storage_map_container = hopscotch_set<
            storage_key_idx_pair,
            key_idx_hasher,
            key_idx_equal,
            key_idx_less,
            _STD allocator<storage_key_idx_pair>,
            4,
            true
        >;

    private:
        vector<storage_page_type> _pages;

        constexpr FORCEINLINE static page_index_type unmask_page_index(cref<index_type> masked_) noexcept {
            return static_cast<page_index_type>((masked_ & index_page_mask) >> index_page_shift);
        }

        constexpr FORCEINLINE static value_index_type unmask_value_index(cref<index_type> masked_) noexcept {
            return static_cast<value_index_type>(masked_ & index_value_mask);
        }

        constexpr FORCEINLINE static void unmask(
            IN cref<index_type> masked_,
            OUT ref<page_index_type> page_,
            OUT ref<value_index_type> value_
        ) noexcept {
            page_ = unmask_page_index(masked_);
            value_ = unmask_value_index(masked_);
        }

    private:
        storage_map_container _mapping;

    public:
        /**
         * Default constructor
         *
         * @author Julius
         * @date 05.08.2021
         */
        scene_node_storage() :
            _pages(),
            _mapping() {}

        /**
         * Copy Constructor
         *
         * @author Julius
         * @date 05.08.2021
         */
        scene_node_storage(const_reference_type) = delete;

        /**
         * Move Constructor
         *
         * @author Julius
         * @date 05.08.2021
         *
         * @param  other_ The other.
         */
        scene_node_storage(mref<this_type> other_) noexcept :
            _pages(_STD move(other_._pages)),
            _mapping(_STD move(other_._mapping)) {}

        /**
         * Destructor
         *
         * @author Julius
         * @date 05.08.2021
         */
        ~scene_node_storage() = default;

    public:
        /**
         * Copy Assignment operator
         *
         * @author Julius
         * @date 05.08.2021
         *
         * @returns A shallow copy of this.
         */
        reference_type operator=(const_reference_type) = delete;

        /**
         * Move Assignment operator
         *
         * @author Julius
         * @date 05.08.2021
         *
         * @param  other_ The other.
         *
         * @returns A shallow copy of this.
         */
        reference_type operator=(mref<this_type> other_) noexcept {
            if (this != &other_) {
                _pages = _STD move(other_._pages);
                _mapping = _STD move(other_._mapping);
            }

            return *this;
        }

    private:
        [[nodiscard]] index_type pages_insert_pair(cref<KeyType> key_, mref<ValueType> value_) noexcept(
            _STD is_nothrow_move_constructible_v<ValueType>
        ) {
            auto possiblePage = _STD find_if(_pages.begin(), _pages.end(), [](cref<storage_page_type> page_) {
                return page_.can_store();
            });

            /**
             * Check whether there is a page with space left to store data
             */
            if (possiblePage == _pages.end()) {

                /**
                 * If no page was found, create a new page and retry
                 */
                _pages.push_back(storage_page_type {});

                /**
                 * Recursive retry
                 */
                return pages_insert_pair(key_, _STD forward<ValueType>(value_));
            }

            /**
             * Get mutation reference to storage page
             */
            typename storage_page_type::reference_type page = *possiblePage;

            /**
             * Emplace data to page
             */
            value_index_type vidx;
            [[maybe_unused]] const bool succeeded = page.insert(key_, _STD forward<ValueType>(value_), vidx);

            const page_index_type pidx = _STD distance(_pages.begin(), possiblePage);

            /**
             * Compose masked index_type
             */
            return (index_type { vidx } | (index_type { pidx } << index_page_shift));
        }

    public:
        template <typename ValueType_ = ValueType, _STD enable_if_t<_STD is_default_constructible_v<ValueType_>>>
        [[nodiscard]] _STD pair<ptr<ValueType>, bool> insert(cref<KeyType> key_) {
            auto ir = _mapping.emplace(storage_key_idx_pair {
                key_,
                0
            });

            ref<storage_key_idx_pair> skip = ir.first.value();

            /**
             * Check whether emplace at mapping failed
             */
            ptr<ValueType> pptr;
            if (ir.second == false) {
                /**
                 * If emplace failed, get reference to already existing element
                 */
                pptr = _pages[unmask_page_index(skip.idx)].value_at(unmask_value_index(skip.idx));
            } else {
                /**
                 * If emplace succeeded, default construct element in page compressed storage
                 */
                const index_type pidx = page_insert_pair(key_, {});

                /**
                 * Store pidx to mapping entry
                 */
                skip.idx = pidx;

                /**
                 * Get reference to existing element
                 */
                pptr = _pages[unmask_page_index(skip.idx)].value_at(unmask_value_index(skip.idx));
            }

            return _STD make_pair(pptr, ir.second);
        }

        [[nodiscard]] _STD pair<ptr<ValueType>, bool> insert(cref<KeyType> key_, mref<ValueType> value_) {
            auto ir = _mapping.emplace(storage_key_idx_pair {
                key_,
                0
            });

            ref<storage_key_idx_pair> skip = ir.first.value();

            /**
             * Check whether emplace at mapping failed
             */
            ptr<ValueType> pptr;
            if (ir.second == false) {
                /**
                 * If emplace failed, get reference to already existing element
                 */
                pptr = _pages[unmask_page_index(skip.idx)].value_at(unmask_value_index(skip.idx));
            } else {
                /**
                 * If emplace succeeded, move construct element in page compressed storage
                 */
                const index_type pidx = pages_insert_pair(key_, _STD forward<ValueType>(value_));

                /**
                 * Store pidx to mapping entry
                 */
                skip.idx = pidx;

                /**
                 * Get reference to stored element
                 */
                pptr = _pages[unmask_page_index(skip.idx)].value_at(unmask_value_index(skip.idx));
            }

            return _STD make_pair(pptr, ir.second);
        }

        [[nodiscard]] ref<ValueType> insert_or_assign(cref<KeyType> key_, mref<ValueType> value_) {
            auto ir = _mapping.emplace(storage_key_idx_pair {
                key_,
                0
            });

            ref<storage_key_idx_pair> skip = ir.first.value();

            /**
             * Check whether emplace at mapping failed
             */
            ptr<ValueType> pptr;
            if (ir.second == false) {
                /**
                 * If emplace failed, replace already existing element
                 */
                ref<storage_page_type> page = _pages[unmask_page_index(skip.idx)];

                /**
                 * Replace existing key-value pair
                 */
                page.replace(unmask_value_index(skip.idx), key_, _STD forward<ValueType>(value_));

                /**
                 * Get reference to stored element
                 */
                pptr = page.value_at(unmask_value_index(skip.idx));
            } else {
                /**
                 * If emplace succeeded, move construct element in page compressed storage
                 */
                const index_type pidx = page_insert_pair(key_, _STD forward<ValueType>(value_));

                /**
                 * Store pidx to mapping entry
                 */
                skip.idx = pidx;

                /**
                 * Get reference to stored element
                 */
                pptr = _pages[unmask_page_index(skip.idx)].value_at(unmask_value_index(skip.idx));
            }

            return *pptr;
        }

        /**
         * Gets a const pointer to a ValueType element, if key exists, otherwise nullptr
         *
         * @author Julius
         * @date 16.08.2021
         *
         * @param  key_ The key to get.
         *
         * @returns A const ptr&lt;ValueType&gt;
         */
        [[nodiscard]] const ptr<ValueType> get(cref<KeyType> key_) const noexcept {
            typename storage_map_container::const_iterator it = _mapping.find(storage_key_idx_pair { key_, 0 });

            if (it == _mapping.cend()) {
                return nullptr;
            }

            return _pages[unmask_page_index(it.value().idx)].value_at(unmask_value_index(it.value().idx));
        }

        /**
         * Gets a pointer to a ValueType element, if key exists, otherwise nullptr
         *
         * @author Julius
         * @date 16.08.2021
         *
         * @param  key_ The key to get.
         *
         * @returns A ptr&lt;ValueType&gt;
         */
        [[nodiscard]] ptr<ValueType> get(cref<KeyType> key_) noexcept {
            typename storage_map_container::const_iterator it = _mapping.find(storage_key_idx_pair { key_, 0 });

            if (it == _mapping.end()) {
                return nullptr;
            }

            return _pages[unmask_page_index(it.value().idx)].value_at(unmask_value_index(it.value().idx));
        }

        /**
         * Erases a key-value pair from hybrid structure, otherwise throw
         *
         * @author Julius
         * @date 16.08.2021
         *
         * @exception _STD Thrown when a Standard error condition occurs.
         *
         * @param  key_ The key.
         */
        void erase(cref<KeyType> key_) {
            typename storage_map_container::const_iterator it = _mapping.find(storage_key_idx_pair { key_, 0 });

            /**
             * If key_ does not exist, throw
             */
            if (it == _mapping.end()) {
                throw _STD out_of_range(
                    "Can not erase element from storage, while key_ does not exist in hybrid structure");
            }

            /**
             * Get mutation reference to storage page
             */
            ref<storage_page_type> page = _pages[unmask_page_index(it.value().idx)];

            /**
             * Erase key-value pair from storage page
             */
            page.erase(unmask_value_index(it.value().idx));

            /**
             * Tidy mapping
             */
            _mapping.erase(it);
        }

    private:
        template <bool Const>
        class storage_key_value_iterator final {
        private:
            using this_type = storage_key_value_iterator<Const>;

            using key_type = KeyType;
            using value_type = ValueType;

            using storage_page_type = _STD conditional_t<Const, const storage_page_type, storage_page_type>;
            using storage_page_iterator_type = storage_page_type::page_key_value_iterator;

        public:
            using iterator_category = _STD forward_iterator_tag;
        };

    public:
        using iterator = storage_key_value_iterator<false>;
        using const_iterator = storage_key_value_iterator<true>;
    };
}
