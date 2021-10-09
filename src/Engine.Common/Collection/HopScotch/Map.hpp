#pragma once

#include <map>

#include "HopScotch.hpp"

namespace ember::hopscotch {

    template <
        typename KeyType_,
        typename ValueType_,
        class HashType_ = _STD hash<KeyType_>,
        class EqualType_ = _STD equal_to<KeyType_>,
        class CompareType_ = _STD less<KeyType_>,
        class AllocatorType_ = _STD allocator<_STD pair<const KeyType_, ValueType_>>,
        u8 NeighborhoodSize_ = 4,
        bool StoreHash_ = false,
        IsGrowthPolicy GrowthPolicy_ = exp2_growth_policy<>>
    class map {
    private:
        using hsh_type = hopscotch_hash<
            const KeyType_,
            ValueType_,
            HashType_,
            EqualType_,
            AllocatorType_,
            NeighborhoodSize_,
            StoreHash_,
            GrowthPolicy_,
            std::map<const KeyType_, ValueType_, CompareType_, AllocatorType_>
        >;

    public:
        using this_type = map<KeyType_, ValueType_, HashType_, EqualType_, CompareType_, AllocatorType_,
            NeighborhoodSize_, StoreHash_, GrowthPolicy_>;

        using value_type = typename hsh_type::value_type;
        using reference_type = ref<value_type>;
        using const_reference_type = cref<value_type>;

        using iterator = typename hsh_type::iterator;
        using const_iterator = typename hsh_type::const_iterator;

        using size_type = typename hsh_type::size_type;

    public:
        /**
         * Default constructor
         *
         * @author Julius
         * @date 04.10.2021
         */
        map() :
            map(init_bucket_count) {}

        /**
         * Constructor
         *
         * @author Julius
         * @date 04.10.2021
         *
         * @param  capacity_ The capacity.
         * @param  hash_ (Optional) The hash.
         * @param  equal_ (Optional) The equal.
         * @param  allocator_ (Optional) The allocator.
         */
        map(size_type capacity_, HashType_ hash_ = {}, EqualType_ equal_ = {}, AllocatorType_ allocator_ = {}) :
            _hsh(capacity_, hash_, equal_, allocator_, init_max_load_factor) {}

        /**
         * Copy Constructor
         *
         * @author Julius
         * @date 04.10.2021
         *
         * @param  other_ The other.
         */
        map(cref<this_type> other_) = default;

        /**
         * Move Constructor
         *
         * @author Julius
         * @date 04.10.2021
         *
         * @param  other_ The other.
         */
        map(mref<this_type> other_) noexcept = default;

        /**
         * Destructor
         *
         * @author Julius
         * @date 04.10.2021
         */
        ~map() = default;

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
                _hsh = other_._hsh;
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
                _hsh = _STD move(other_._hsh);
            }

            return *this;
        }

    private:
        /** The internal hopscotch hash */
        hsh_type _hsh;

    public:
        [[nodiscard]] iterator begin() noexcept {
            return _hsh.begin();
        }

        [[nodiscard]] const_iterator cbegin() const noexcept {
            return _hsh.cbegin();
        }

        [[nodiscard]] iterator end() noexcept {
            return _hsh.end();
        }

        [[nodiscard]] const_iterator cend() const noexcept {
            return _hsh.cend();
        }

    public:
        [[nodiscard]] size_type size() const noexcept {
            return _hsh.size();
        }

        [[nodiscard]] size_type capacity() const noexcept {
            return _hsh.capacity();
        }

        [[nodiscard]] bool empty() const noexcept {
            return _hsh.empty();
        }

    public:
        auto insert(cref<value_type> value_) {
            return _hsh.insert(value_);
        }

        auto insert(mref<value_type> value_) {
            return _hsh.insert(_STD forward<value_type>(value_));
        }

        auto insert_or_assign(cref<value_type> value_) {
            return _hsh.insert_or_assign(_STD forward<value_type>(value_));
        }

        auto insert_or_assign(mref<value_type> value_) {
            return _hsh.insert_or_assign(_STD forward<value_type>(value_));
        }

        auto emplace(cref<value_type> value_) {
            return _hsh.emplace(value_);
        }

        auto emplace(mref<value_type> value_) {
            return _hsh.emplace(_STD forward<value_type>(value_));
        }

    public:
        iterator erase(iterator where_) {
            return _hsh.erase(where_);
        }

        iterator erase(const_iterator where_) {
            return _hsh.erase(where_);
        }

    public:
        [[nodiscard]] size_type count(cref<KeyType_> key_) const {
            return _hsh.count(key_);
        }

        [[nodiscard]] iterator find(cref<KeyType_> key_) {
            return _hsh.find(key_);
        }

        [[nodiscard]] const_iterator find(cref<KeyType_> key_) const {
            return _hsh.find(key_);
        }

        [[nodiscard]] bool contains(cref<KeyType_> key_) const {
            return _hsh.contains(key_);
        }

    public:
        void clear() noexcept {
            return _hsh.clear();
        }

        void reserve(size_type count_) {
            return _hsh.reserve(count_);
        }
    };

}
