#pragma once

#include <algorithm>
#include <ranges>

#include "Hash.hpp"
#include "Table.hpp"

namespace hg {
	template <typename Type_, size_t Size_, typename Hash_ = CompileHash<Type_>>
	struct CompileTableHashKey :
		public Hash_ {
		using type = CompileTableHashKey<Type_, Size_, Hash_>;
		using value_type = Type_;

	public:
		constexpr CompileTableHashKey(const size_t hash_, const value_type& value_) :
			hash(hash_),
			data(value_) {}

		constexpr CompileTableHashKey(const value_type& value_) :
			hash(static_cast<Hash_&>(*this)(value_)),
			data(value_) {}

		template <typename Ty_ = Type_> requires std::is_same_v<Ty_, Type_> && std::is_copy_constructible_v<Ty_>
		constexpr CompileTableHashKey(const Ty_& other_) noexcept :
			hash(other_.hash),
			data(other_.data) {}

		template <typename Ty_ = Type_> requires std::is_same_v<Ty_, Type_> && std::is_move_constructible_v<Ty_>
		constexpr CompileTableHashKey(Ty_&& other_) noexcept :
			hash(other_.hash),
			data(std::move(other_.data)) {}

		constexpr ~CompileTableHashKey() noexcept = default;

	public:
		size_t hash;
		value_type data;

	public:
		template <typename Ty_ = type> requires
			std::is_same_v<Ty_, type> && std::is_copy_assignable_v<typename Ty_::value_type>
		constexpr type& operator=(const Ty_& other_) noexcept {
			if (std::addressof(other_) != this) {
				hash = other_.hash;
				data = other_.data;
			}
			return *this;
		}

		template <typename Ty_ = type> requires
			std::is_same_v<Ty_, type> && std::is_move_assignable_v<typename Ty_::value_type>
		constexpr type& operator=(Ty_&& other_) noexcept {
			if (std::addressof(other_) != this) {
				hash = other_.hash;
				data = std::move(other_.data);
			}
			return *this;
		}

	public:
		constexpr bool operator<(const type& other_) const noexcept {
			return hash < other_.hash;
		}

		constexpr bool operator>(const type& other_) const noexcept {
			return hash > other_.hash;
		}

		constexpr bool operator<=(const type& other_) const noexcept {
			return hash <= other_.hash;
		}

		constexpr bool operator>=(const type& other_) const noexcept {
			return hash >= other_.hash;
		}

		constexpr bool operator==(const type& other_) const noexcept {
			return hash == other_.hash;
		}

		constexpr bool operator!=(const type& other_) const noexcept {
			return hash != other_.hash;
		}

		[[nodiscard]] constexpr std::strong_ordering operator<=>(const type& other_) const noexcept {
			return hash <=> other_.hash;
		}
	};

	template <
		typename Key_,
		typename Value_,
		size_t Size_,
		typename Hash_ = CompileHash<Key_>>
	class CompileHashMap {
	public:
		using this_type = CompileHashMap<Key_, Value_, Size_, Hash_>;

		using key_type = typename CompileTableHashKey<Key_, Size_, Hash_>::type;
		using value_type = Value_;

	private:
		struct CompileMapComparator {
			using node_type = CompileTableNode<key_type, value_type>;

			[[nodiscard]] constexpr bool operator()(cref<node_type> left_, cref<node_type> right_) const noexcept {
				return std::less {}(left_->first, right_->first);
			}
		};

	public:
		using comparator = CompileMapComparator;
		using table_type = CompileTable<CompileTableNode<key_type, value_type>, Size_, comparator>;

	public:
		constexpr CompileHashMap(const typename table_type::node_type (&pairs_)[Size_]) :
			_table(pairs_) {}

		constexpr CompileHashMap(const std::pair<key_type, value_type> (&pairs_)[Size_]) :
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

			// Threshold where to switch between linear search and binary hash probing
			constexpr size_t threshold = 56;
			if constexpr (Size_ < threshold) {
				return std::ranges::find_if(
					_table,
					[key_](cref<typename table_type::node_type> node_) {
						return node_->first == key_;
					}
				);
			} else {
				// TODO: Rework hashing for constexpr tables
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

		[[nodiscard]] constexpr auto find(const typename key_type::value_type& key_) const noexcept {
			return find(key_type { key_ });
		}

		[[nodiscard]] constexpr bool contains(const key_type& key_) const noexcept {
			return find(key_) != end();
		}

		[[nodiscard]] constexpr bool contains(const typename key_type::value_type& key_) const noexcept {
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

		[[nodiscard]] constexpr size_t count(const typename key_type::value_type& key_) const noexcept {
			return count(key_type { key_ });
		}

	public:
		[[nodiscard]] constexpr auto operator[](const key_type& key_) const noexcept {
			return find(key_);
		}

		[[nodiscard]] constexpr auto operator[](const typename key_type::value_type& key_) const noexcept {
			return find(key_);
		}
	};

	/**/

	template <typename Key_, typename Value_, typename Hash_ = CompileHash<Key_>, size_t Size_>
	[[nodiscard]] constexpr auto make_compile_hash_map(
		const std::pair<typename CompileHashMap<Key_, Value_, Size_, Hash_>::key_type, Value_> (&pairs_)[Size_]
	) {
		return CompileHashMap<Key_, Value_, Size_, Hash_> { pairs_ };
	}

	template <typename Key_, typename Value_, typename Hash_ = CompileHash<Key_>, size_t Size_>
	[[nodiscard]] constexpr auto make_compile_hash_map(
		const typename CompileHashMap<Key_, Value_, Size_, Hash_>::table_type::node_type (&nodes_)[Size_]
	) {
		return CompileHashMap<Key_, Value_, Size_, Hash_> { nodes_ };
	}

	template <typename Key_, typename Value_, typename Hash_ = CompileHash<Key_>, typename... PairTypes_>
	[[nodiscard]] constexpr auto make_compile_hash_map(
		const PairTypes_... pairs_
	) {
		using map_type = CompileHashMap<Key_, Value_, sizeof...(pairs_), Hash_>;

		static_assert(
			sizeof...(pairs_) > 0,
			"Requires at least one pair to construct compile map via vardiac factoring."
		);

		std::pair<typename map_type::key_type, Value_> tmp[sizeof...(pairs_)] {
			{ { pairs_.first }, { pairs_.second } }...
		};
		return CompileHashMap<Key_, Value_, sizeof...(pairs_), Hash_> { tmp };
	}
}
