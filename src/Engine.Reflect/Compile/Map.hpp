#pragma once

#include "Table.hpp"
#include "Relation.hpp"

namespace hg {
    template <typename Type_>
    struct CompileTableKey {
        using type = Type_;
    };

    template <
        typename Key_,
        typename Value_,
        size_t Size_,
        typename Comparator_ = CompileLess<Key_>>
    class CompileMap {
    public:
        using this_type = CompileMap<Key_, Value_, Size_, Comparator_>;

        using key_type = typename CompileTableKey<Key_>::type;
        using value_type = Value_;

    private:
        template <typename Type_>
        struct CompileMapComparator :
            public Type_ {
            using underlying_comparator = Type_;
            using node_type = CompileTableNode<key_type, value_type>;

            [[nodiscard]] constexpr const underlying_comparator& underlying() const noexcept {
                return static_cast<const underlying_comparator&>(*this);
            }

            [[nodiscard]] constexpr bool operator()(cref<node_type> left_, cref<node_type> right_) const noexcept {
                return underlying()(left_->first, right_->first);
            }
        };

    public:
        using comparator = CompileMapComparator<Comparator_>;
        using table_type = CompileTable<CompileTableNode<key_type, value_type>, Size_, comparator>;

    public:
        constexpr CompileMap(const typename table_type::node_type (&pairs_)[Size_]) :
            _table(pairs_) {}

        constexpr CompileMap(const std::pair<key_type, value_type> (&pairs_)[Size_]) :
            _table(pairs_) {}

    private:
        table_type _table;

    public:
        [[nodiscard]] constexpr auto begin() const noexcept {
            return _table.begin();
        }

        [[nodiscard]] constexpr auto begin() noexcept {
            return _table.begin();
        }

        [[nodiscard]] constexpr auto cbegin() noexcept {
            return static_cast<const this_type&>(*this).begin();
        }

        [[nodiscard]] constexpr auto end() const noexcept {
            return _table.end();
        }

        [[nodiscard]] constexpr auto end() noexcept {
            return _table.end();
        }

        [[nodiscard]] constexpr auto cend() noexcept {
            return static_cast<const this_type&>(*this).end();
        }

    public:
        [[nodiscard]] constexpr auto empty() const noexcept {
            static_assert(begin() != end());
            return false;
        }

        [[nodiscard]] constexpr auto size() const noexcept {
            return Size_;
        }

    public:
        [[nodiscard]] constexpr auto find(const key_type& key_) const noexcept {

            // Threshold where to switch between linear and binary search
            constexpr size_t threshold = 56;
            if constexpr (Size_ < threshold) {
                return std::ranges::find_if(
                    _table,
                    [key_](cref<typename table_type::node_type> node_) {
                        return node_->first == key_;
                    }
                );
            } else {
                return std::ranges::lower_bound(
                    _table,
                    key_,
                    CompileLess<key_type> {},
                    [](cref<typename table_type::node_type> node_) {
                        return node_->first;
                    }
                );
            }
        }

        [[nodiscard]] constexpr bool contains(const key_type& key_) const noexcept {
            return find(key_) != end();
        }

        [[nodiscard]] constexpr size_t count(const key_type& key_) const noexcept {

            const auto start = find(key_);
            auto it = start;
            const auto end = this->end();
            for (; it != end && (*it)->first == key_; ++it) {
                // __noop();
            }

            return std::distance(start, it);
        }

    public:
        [[nodiscard]] constexpr auto operator[](const key_type& key_) const noexcept {
            return find(key_);
        }
    };

    /**/

    template <
        typename Key_,
        typename Value_,
        typename Comparator_>
    class CompileMap<Key_, Value_, 0, Comparator_> {
    public:
        using this_type = CompileMap<Key_, Value_, 0, Comparator_>;

        using key_type = typename CompileTableKey<Key_>::type;
        using value_type = Value_;

    private:
        template <typename Type_>
        struct CompileMapComparator :
            public Type_ {
            using underlying_comparator = Type_;
            using node_type = CompileTableNode<key_type, value_type>;

            [[nodiscard]] constexpr const underlying_comparator& underlying() const noexcept {
                return static_cast<const underlying_comparator&>(*this);
            }

            [[nodiscard]] constexpr bool operator()(cref<node_type> left_, cref<node_type> right_) const noexcept {
                return underlying()(left_->first, right_->first);
            }
        };

    public:
        using comparator = CompileMapComparator<Comparator_>;
        using table_type = CompileTable<CompileTableNode<key_type, value_type>, 1, comparator>;

    public:
        constexpr CompileMap() noexcept = default;

    public:
        [[nodiscard]] constexpr auto begin() const noexcept {
            return typename table_type::ConstIterator {};
        }

        [[nodiscard]] constexpr auto begin() noexcept {
            return typename table_type::Iterator {};
        }

        [[nodiscard]] constexpr auto cbegin() noexcept {
            return typename table_type::ConstIterator {};
        }

        [[nodiscard]] constexpr auto end() const noexcept {
            return typename table_type::ConstIterator {};
        }

        [[nodiscard]] constexpr auto end() noexcept {
            return typename table_type::Iterator {};
        }

        [[nodiscard]] constexpr auto cend() noexcept {
            return typename table_type::ConstIterator {};
        }

    public:
        [[nodiscard]] constexpr auto empty() const noexcept {
            return true;
        }

        [[nodiscard]] constexpr auto size() const noexcept {
            return 0;
        }

    public:
        [[nodiscard]] constexpr auto find([[maybe_unused]] const key_type& key_) const noexcept {
            return typename table_type::ConstIterator {};
        }

        [[nodiscard]] constexpr bool contains([[maybe_unused]] const key_type& key_) const noexcept {
            return false;
        }

        [[nodiscard]] constexpr size_t count([[maybe_unused]] const key_type& key_) const noexcept {
            return 0;
        }

    public:
        [[nodiscard]] constexpr auto operator[]([[maybe_unused]] const key_type& key_) const noexcept {
            return typename table_type::ConstIterator {};
        }
    };

    /**/

    template <typename Key_, typename Value_, typename Comparator_ = CompileLess<Key_>, size_t Size_>
    [[nodiscard]] constexpr auto make_compile_map(
        const std::pair<Key_, Value_> (&pairs_)[Size_]
    ) {
        return CompileMap<Key_, Value_, Size_, Comparator_> { pairs_ };
    }

    template <typename Key_, typename Value_, typename Comparator_ = CompileLess<Key_>, size_t Size_>
    [[nodiscard]] constexpr auto make_compile_map(
        const typename CompileMap<Key_, Value_, Size_, Comparator_>::table_type::node_type (&nodes_)[Size_]
    ) {
        return CompileMap<Key_, Value_, Size_, Comparator_> { nodes_ };
    }

    template <typename Key_, typename Value_, typename Comparator_ = CompileLess<Key_>, typename... PairTypes_>
    [[nodiscard]] constexpr auto make_compile_map(
        const PairTypes_... pairs_
    ) {

        static_assert(
            sizeof...(pairs_) > 0,
            "Requires at least one pair to construct compile map via vardiac factoring."
        );

        std::pair<Key_, Value_> tmp[sizeof...(pairs_)] {
            { pairs_ }...
        };
        return CompileMap<Key_, Value_, sizeof...(pairs_), Comparator_> { tmp };
    }
}
