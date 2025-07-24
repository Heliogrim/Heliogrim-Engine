#pragma once

#include <concepts>
#include <iterator>
#include <type_traits>
#include <Engine.Common/__macro.hpp>
#include <Engine.Common/Memory/Address.hpp>
#include <Engine.Common/Meta/Concept.hpp>

#include "Relation.hpp"
#include "Sort.hpp"

namespace hg {
	template <typename Key_, typename Value_>
	using CompilePair = std::pair<Key_, Value_>;

	template <typename Key_, typename Value_>
	struct CompileTableNode {
	public:
		using this_type = CompileTableNode<Key_, Value_>;

		using key_type = Key_;
		using value_type = Value_;

		using mapping = CompilePair<key_type, value_type>;

	public:
		consteval CompileTableNode() noexcept = default;

		template <template <typename, typename> typename Type_> requires
			std::is_same_v<Type_<Key_, Value_>, mapping>
		consteval CompileTableNode(const Type_<Key_, Value_> data_) noexcept :
			data(data_) {}

		template <template <typename, typename> typename Type_> requires
			(not std::is_same_v<Type_<Key_, Value_>, mapping>) &&
			std::is_nothrow_convertible_v<Type_<Key_, Value_>, mapping>
		consteval CompileTableNode(const Type_<Key_, Value_> data_) noexcept :
			data { data_ } {}

		consteval CompileTableNode(const this_type& other_) noexcept :
			data(other_.data) {}

		consteval CompileTableNode(this_type&& other_) noexcept :
			data(std::move(other_.data)) {}

		constexpr ~CompileTableNode() noexcept = default;

	public:
		mapping data;

	public:
		constexpr this_type& operator=(const this_type& other_) noexcept {
			if (std::addressof(other_) != this) {
				data = other_.data;
			}
			return *this;
		}

		constexpr this_type& operator=(this_type&& other_) noexcept {
			if (std::addressof(other_) != this) {
				data = std::move(other_.data);
			}
			return *this;
		}

	public:
		constexpr void swap(this_type& other_) noexcept {
			CompileSwap<key_type> {}(data.first, other_.data.first);
			CompileSwap<value_type> {}(data.second, other_.data.second);
		}

	public:
		constexpr bool operator<(const this_type& other_) const noexcept {
			return data.first < other_.data.first;
		}

		constexpr bool operator<(const key_type& other_) const noexcept {
			return data.first < other_;
		}

	public:
		constexpr const auto& operator*() const noexcept {
			return data;
		}

		constexpr const auto* operator->() const noexcept {
			return &data;
		}
	};

	/**/

	template <typename NodeType_, size_t Size_, typename Comparator_>
	class CompileTable {
	public:
		using this_type = CompileTable<NodeType_, Size_, Comparator_>;

		using node_type = NodeType_;
		constexpr static size_t size = Size_;

	private:
		template <size_t... Idx_>
		consteval CompileTable(const node_type (&entries_)[Size_], std::index_sequence<Idx_...>) noexcept :
			_data { { entries_[Idx_] }... } {
			static_assert(sizeof...(Idx_) == Size_, "Size of index sequence has to match the element count.");
			//compile_sort(std::begin(_data), std::end(_data), Comparator_ {});
		}

		template <typename Type_, size_t... Idx_> requires std::is_nothrow_convertible_v<Type_, NodeType_>
		consteval CompileTable(const Type_ (&values_)[Size_], std::index_sequence<Idx_...>) noexcept :
			_data { (node_type { values_[Idx_] })... } {
			static_assert(sizeof...(Idx_) == Size_, "Size of index sequence has to match the element count.");
			compile_sort(begin(), end(), Comparator_ {});
		}

	public:
		consteval CompileTable(const node_type (&entries_)[Size_]) noexcept :
			CompileTable(entries_, std::make_index_sequence<Size_>()) {}

		template <typename Type_> requires std::is_nothrow_convertible_v<Type_, NodeType_>
		consteval CompileTable(const Type_ (&values_)[Size_]) noexcept :
			CompileTable(values_, std::make_index_sequence<Size_>()) {}

	private:
		node_type _data[Size_];

	public:
		class Iterator;
		class ConstIterator;

		class Iterator {
		public:
			using this_type = Iterator;

			#ifdef __cpp_lib_concepts
			using iterator_concept = std::random_access_iterator_tag;
			#endif

			using iterator_category = std::random_access_iterator_tag;

			using value_type = node_type;
			using reference = node_type&;
			using const_reference = const node_type&;

			using pointer = node_type*;
			using const_pointer = const node_type*;

			using size_type = std::size_t;
			using difference_type = std::ptrdiff_t;

		public:
			constexpr Iterator() noexcept = default;

			constexpr explicit Iterator(const pointer cursor_) noexcept :
				_cursor(cursor_) {}

			constexpr Iterator(this_type&& other_) noexcept = default;

			constexpr Iterator(const this_type& other_) noexcept = default;

			constexpr ~Iterator() noexcept = default;

		private:
			pointer _cursor = nullptr;

		public:
			constexpr this_type& operator=(const this_type& other_) noexcept {
				if (std::addressof(other_) != this) {
					_cursor = other_._cursor;
				}
				return *this;
			}

			constexpr this_type& operator=(this_type&& other_) noexcept {
				if (std::addressof(other_) != this) {
					_cursor = other_._cursor;
				}
				return *this;
			}

		public:
			[[nodiscard]] constexpr const_reference operator*() const noexcept {
				return *_cursor;
			}

			[[nodiscard]] constexpr reference operator*() noexcept {
				return *_cursor;
			}

			[[nodiscard]] constexpr const_pointer operator->() const noexcept {
				return _cursor;
			}

		public:
			[[nodiscard]] constexpr const_reference operator[](const difference_type offset_) const noexcept {
				return _cursor[offset_];
			}

			[[nodiscard]] constexpr reference operator[](const difference_type offset_) noexcept {
				return _cursor[offset_];
			}

		public:
			constexpr this_type& operator++() {
				GUARD_UNCHECKED_LLVL(++_cursor);
				return *this;
			}

			[[nodiscard]] constexpr this_type operator++(int) {
				constexpr auto tmp { *this };
				GUARD_UNCHECKED_LLVL(++_cursor);
				return tmp;
			}

			constexpr this_type& operator+=(difference_type offset_) noexcept {
				GUARD_UNCHECKED_LLVL(_cursor += offset_);
				return *this;
			}

			[[nodiscard]] constexpr this_type operator+(difference_type offset_) const noexcept {
				this_type tmp { *this };
				GUARD_UNCHECKED_LLVL(tmp += offset_);
				return tmp;
			}

			constexpr this_type& operator--() {
				GUARD_UNCHECKED_LLVL(--_cursor);
				return *this;
			}

			[[nodiscard]] constexpr this_type operator--(int) {
				constexpr auto tmp { *this };
				GUARD_UNCHECKED_LLVL(--_cursor);
				return tmp;
			}

			constexpr this_type& operator-=(difference_type offset_) noexcept {
				GUARD_UNCHECKED_LLVL(_cursor -= offset_);
				return *this;
			}

			[[nodiscard]] constexpr this_type operator-(difference_type offset_) const noexcept {
				this_type tmp { *this };
				GUARD_UNCHECKED_LLVL(tmp -= offset_);
				return tmp;
			}

		public:
			[[nodiscard]] constexpr difference_type operator-(const this_type& other_) const noexcept {
				return _cursor - other_._cursor;
			}

		public:
			[[nodiscard]] constexpr bool operator==(const this_type& other_) const noexcept {
				return _cursor == other_._cursor;
			}

			[[nodiscard]] constexpr bool operator==(const ConstIterator& other_) const noexcept;

			[[nodiscard]] constexpr bool operator!=(const this_type& other_) const noexcept {
				return _cursor != other_._cursor;
			}

			[[nodiscard]] constexpr bool operator!=(const ConstIterator& other_) const noexcept;

			[[nodiscard]] constexpr std::strong_ordering operator<=>(const this_type& other_) const noexcept {
				return ::hg::unfancy(_cursor) <=> ::hg::unfancy(other_._cursor);
			}

			[[nodiscard]] constexpr std::strong_ordering operator<=>(const ConstIterator& other_) const noexcept;
		};

		class ConstIterator {
		public:
			using this_type = ConstIterator;

			#ifdef __cpp_lib_concepts
			using iterator_concept = std::random_access_iterator_tag;
			#endif

			using iterator_category = std::random_access_iterator_tag;

			using value_type = node_type;
			using reference = node_type&;
			using const_reference = const node_type&;

			using pointer = node_type*;
			using const_pointer = const node_type*;

			using size_type = std::size_t;
			using difference_type = std::ptrdiff_t;

		public:
			constexpr ConstIterator() noexcept = default;

			constexpr explicit ConstIterator(const const_pointer cursor_) noexcept :
				_cursor(cursor_) {}

			constexpr ConstIterator(this_type&& other_) noexcept = default;

			constexpr ConstIterator(const this_type& other_) noexcept = default;

			constexpr ~ConstIterator() noexcept = default;

		private:
			const_pointer _cursor = nullptr;

		public:
			constexpr this_type& operator=(const this_type& other_) noexcept {
				if (std::addressof(other_) != this) {
					_cursor = other_._cursor;
				}
				return *this;
			}

			constexpr this_type& operator=(this_type&& other_) noexcept {
				if (std::addressof(other_) != this) {
					_cursor = other_._cursor;
				}
				return *this;
			}

		public:
			[[nodiscard]] constexpr const_reference operator*() const noexcept {
				return *_cursor;
			}

			[[nodiscard]] constexpr const_reference operator*() noexcept {
				return *_cursor;
			}

			[[nodiscard]] constexpr const_pointer operator->() const noexcept {
				return _cursor;
			}

		public:
			[[nodiscard]] constexpr const_reference operator[](const difference_type offset_) const noexcept {
				return _cursor[offset_];
			}

			[[nodiscard]] constexpr const_reference operator[](const difference_type offset_) noexcept {
				return _cursor[offset_];
			}

		public:
			constexpr this_type& operator++() {
				GUARD_UNCHECKED_LLVL(++_cursor);
				return *this;
			}

			[[nodiscard]] constexpr this_type operator++(int) {
				constexpr auto tmp { *this };
				GUARD_UNCHECKED_LLVL(++_cursor);
				return tmp;
			}

			constexpr this_type& operator+=(difference_type offset_) noexcept {
				_cursor += offset_;
				return *this;
			}

			[[nodiscard]] constexpr this_type operator+(difference_type offset_) const noexcept {
				this_type tmp { *this };
				tmp += offset_;
				return tmp;
			}

			constexpr this_type& operator--() {
				GUARD_UNCHECKED_LLVL(--_cursor);
				return *this;
			}

			[[nodiscard]] constexpr this_type operator--(int) {
				constexpr auto tmp { *this };
				GUARD_UNCHECKED_LLVL(--_cursor);
				return tmp;
			}

			constexpr this_type& operator-=(difference_type offset_) noexcept {
				GUARD_UNCHECKED_LLVL(_cursor -= offset_);
				return *this;
			}

			[[nodiscard]] constexpr this_type operator-(difference_type offset_) const noexcept {
				this_type tmp { *this };
				GUARD_UNCHECKED_LLVL(tmp -= offset_);
				return tmp;
			}

		public:
			[[nodiscard]] constexpr difference_type operator-(const this_type& other_) const noexcept {
				return _cursor - other_._cursor;
			}

		public:
			[[nodiscard]] constexpr bool operator==(const this_type& other_) const noexcept {
				return _cursor == other_._cursor;
			}

			[[nodiscard]] constexpr bool operator==(const Iterator& other_) const noexcept;

			[[nodiscard]] constexpr bool operator!=(const this_type& other_) const noexcept {
				return _cursor != other_._cursor;
			}

			[[nodiscard]] constexpr bool operator!=(const Iterator& other_) const noexcept;

			[[nodiscard]] constexpr std::strong_ordering operator<=>(const this_type& other_) const noexcept {
				return ::hg::unfancy(_cursor) <=> ::hg::unfancy(other_._cursor);
			}

			[[nodiscard]] constexpr std::strong_ordering operator<=>(const Iterator& other_) const noexcept;
		};

	public:
		[[nodiscard]] constexpr ConstIterator begin() const noexcept {
			return ConstIterator { _data };
		}

		[[nodiscard]] constexpr Iterator begin() noexcept {
			return Iterator { _data };
		}

		[[nodiscard]] constexpr ConstIterator end() const noexcept {
			GUARD_UNCHECKED_LLVL(return ConstIterator { _data + Size_ });
		}

		[[nodiscard]] constexpr Iterator end() noexcept {
			GUARD_UNCHECKED_LLVL(return Iterator { _data + Size_ });
		}
	};

	/**/

	template <typename NodeType_, size_t Size_, typename Comparator_>
	constexpr bool CompileTable<NodeType_, Size_, Comparator_>::Iterator::operator==(
		const ConstIterator& other_
	) const noexcept {
		return _cursor == &(*other_);
	}

	template <typename NodeType_, size_t Size_, typename Comparator_>
	constexpr bool CompileTable<NodeType_, Size_, Comparator_>::Iterator::operator!=(
		const ConstIterator& other_
	) const noexcept {
		return _cursor != &(*other_);
	}

	template <typename NodeType_, size_t Size_, typename Comparator_>
	constexpr std::strong_ordering CompileTable<NodeType_, Size_, Comparator_>::Iterator::operator<=>(
		const ConstIterator& other_
	) const noexcept {
		return _cursor <=> &(*other_);
	}

	template <typename NodeType_, size_t Size_, typename Comparator_>
	constexpr bool CompileTable<NodeType_, Size_, Comparator_>::ConstIterator::operator==(
		const Iterator& other_
	) const noexcept {
		return _cursor == &(*other_);
	}

	template <typename NodeType_, size_t Size_, typename Comparator_>
	constexpr bool CompileTable<NodeType_, Size_, Comparator_>::ConstIterator::operator!=(
		const Iterator& other_
	) const noexcept {
		return _cursor != &(*other_);
	}

	template <typename NodeType_, size_t Size_, typename Comparator_>
	constexpr std::strong_ordering CompileTable<NodeType_, Size_, Comparator_>::ConstIterator::operator<=>(
		const Iterator& other_
	) const noexcept {
		return _cursor <=> &(*other_);
	}
}
