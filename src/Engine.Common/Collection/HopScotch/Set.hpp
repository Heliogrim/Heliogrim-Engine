#pragma once

#include <set>

#include "HopScotch.hpp"

namespace ember::hopscotch {
    template <
        typename ValueType_,
        class HashType_ = _STD hash<ValueType_>,
        class EqualType_ = _STD equal_to<ValueType_>,
        class CompareType_ = _STD less<ValueType_>,
        class AllocatorType_ = _STD allocator<ValueType_>,
        u8 NeighborhoodSize_ = 4,
        bool StoreHash_ = false,
        IsGrowthPolicy GrowthPolicy_ = exp2_growth_policy<>>
    class set {
    private:
        using hsh_type = hopscotch_hash<
            ValueType_,
            void,
            HashType_,
            EqualType_,
            AllocatorType_,
            NeighborhoodSize_,
            StoreHash_,
            GrowthPolicy_,
            std::set<ValueType_>
        >;

    public:
        using this_type = set<ValueType_, HashType_, EqualType_, CompareType_, AllocatorType_,
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
        set() :
            set(init_bucket_count) {}

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
        set(size_type capacity_, HashType_ hash_ = {}, EqualType_ equal_ = {}, AllocatorType_ allocator_ = {}) :
            _hsh(capacity_, hash_, equal_, allocator_, init_max_load_factor) {}

        /**
         * Copy Constructor
         *
         * @author Julius
         * @date 04.10.2021
         *
         * @param  other_ The other.
         */
        set(cref<this_type> other_) = default;

        /**
         * Move Constructor
         *
         * @author Julius
         * @date 04.10.2021
         *
         * @param  other_ The other.
         */
        set(mref<this_type> other_) noexcept = default;

        /**
         * Destructor
         *
         * @author Julius
         * @date 04.10.2021
         */
        ~set() = default;

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
        [[nodiscard]] size_type count(cref<value_type> value_) const {
            return _hsh.count(value_);
        }

        [[nodiscard]] iterator find(cref<value_type> value_) {
            return _hsh.find(value_);
        }

        [[nodiscard]] const_iterator find(cref<value_type> value_) const {
            return _hsh.find(value_);
        }

        [[nodiscard]] bool contains(cref<value_type> value_) const {
            return _hsh.contains(value_);
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
