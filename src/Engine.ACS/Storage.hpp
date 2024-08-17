#pragma once

#include <stdexcept>
#include <unordered_set>
#include <utility>
#include <Engine.Common/Sal.hpp>
#include <Engine.Common/Collection/Vector.hpp>

namespace hg::engine::acs {
	template <typename Ty>
	Ty* construct_inplace(void* destination_) {
		return new(destination_) Ty;
	}

	template <typename Ty>
	Ty* construct_inplace(void* destination_, const Ty& source_) {
		return new(destination_) Ty(source_);
	}

	template <typename Ty>
	Ty* construct_inplace(void* destination_, Ty&& source_) {
		return new(destination_) Ty(std::forward<Ty>(source_));
	}

	template <typename Ty>
	void destruct_inplace(Ty* destination_) {
		destination_->~Ty();
	}

	/**
	 * Forward declaration for hybrid_storage
	 *
	 * @author Julius
	 * @date 23.10.2020
	 *
	 * @tparam KeyType_ Type of the key to index.
	 * @tparam StoredType_ Type of the stored type.
	 * @tparam InvalidKey_ The key value to mark a invalid entry.
	 */
	template <typename KeyType_, typename StoredType_, KeyType_ InvalidKey_>
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
	 * @tparam ValueType Type of the stored type.
	 * @tparam KeyType Type of the identifier type.
	 * @tparam InvalidKey The value to use to mark a invalid key.
	 */
	template <typename ValueType, typename KeyType, KeyType InvalidKey, typename IndexType = size_t>
	class hybrid_storage_page final {
		friend class hybrid_storage<KeyType, ValueType, InvalidKey>;

	public:
		using this_type = hybrid_storage_page<ValueType, KeyType, InvalidKey, IndexType>;
		using this_reference_type = this_type&;
		using this_const_reference_type = const this_type&;

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
		hybrid_storage_page(this_type&& other_) noexcept:
			_mem(std::exchange(other_._mem, nullptr)),
			_keys(std::exchange(other_._keys, nullptr)),
			_values(std::exchange(other_._values, nullptr)),
			_seq(std::move(other_._seq)) {}

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
		 * @date 02.12.2021
		 *
		 * @tparam Args_ The packed parameter type list to construct stored type.
		 *
		 * @param key_ The key to emplace.
		 * @param args_ The typed packed parameter list to construct stored type.
		 * @param idx_ Zero-based index of the.
		 *
		 * @returns True if it succeeds, false if it fails.
		 */
		template <typename... Args_>
		bool emplace(const key_type& key_, Args_&&... args_, index_type& idx_) {
			/**
			 * Get empty slot
			 */
			idx_ = pop_slot();

			/**
			 * Place new key value relation to local storage
			 */
			place<Args_...>(idx_, key_, std::forward<Args_>(args_)...);

			/**
			 * Return whether successful
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
		[[nodiscard]] std::pair<const key_type&, const value_type&> get(const index_type& idx_) const {
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
		[[nodiscard]] std::pair<const key_type&, value_type&> get(const index_type& idx_) {
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
		[[nodiscard]] const key_type& get_key(const index_type& idx_) const {
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
		[[nodiscard]] key_type& get_key(const index_type& idx_) {
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
		[[nodiscard]] const value_type& get_value(const index_type& idx_) const {
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
		[[nodiscard]] value_type& get_value(const index_type& idx_) {
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
		void insert(const index_type& idx_, const key_type& key_, IN value_type&& value_) {
			/**
			 * Check if key exists
			 */
			if (_keys[idx_]) {
				/**
				 * If index in range, override stored element
				 */
				replace(idx_, key_, std::forward<value_type>(value_));
			} else {
				/**
				 * reserve slot from sequence
				 */
				pop_slot(idx_);

				/**
				 * Place the new key and value to local storage
				 */
				place(idx_, key_, std::forward<value_type>(value_));
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
		void erase(const index_type& idx_) {

			// destruct_inplace<key_type>(&_keys[idx_]);
			_keys[idx_] = InvalidKey;
			destruct_inplace<value_type>(&_values[idx_]);

			/**
			 * Find valid minimal sequence where index can merge
			 */
			auto fi = std::find_if(
				_seq.rbegin(),
				_seq.rend(),
				[&idx_](const page_sequence& entry_) {
					return entry_.mergable(idx_);
				}
			);

			/**
			 * If no mergable sequence, create a new one
			 */
			if (fi == _seq.rend()) {
				_seq.push_back({ idx_, idx_ });
				std::sort(_seq.begin(), _seq.end(), page_sequence_range_comparator());
				return;
			}

			/**
			 * Find valid maximal sequence where index can merge
			 */
			auto ri = std::find_if(
				_seq.begin(),
				_seq.end(),
				[&idx_](const page_sequence& entry_) {
					return entry_.mergable(idx_);
				}
			);

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
				std::advance(fi, 1u);
				_seq.erase(fi.base());
				_seq.erase(ri);

				/**
				 * Push the new sequence to list
				 */
				_seq.push_back(v);
				std::sort(_seq.begin(), _seq.end(), page_sequence_range_comparator());
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

			hybrid_value_iterator(IN value_type* ptr_) noexcept:
				_ptr(ptr_) {}

			hybrid_value_iterator(IN value_type* ptr_, const size_t idx_) noexcept:
				_ptr(&ptr_[idx_]) {}

			hybrid_value_iterator(IN hybrid_value_iterator&& other_) noexcept:
				_ptr(std::exchange(other_._ptr, nullptr)) {}

			hybrid_value_iterator(IN const hybrid_value_iterator& other_) noexcept:
				_ptr(other_._ptr) {}

			hybrid_value_iterator& operator++() noexcept {
				return ++_ptr, *this;
			}

			hybrid_value_iterator& operator--() noexcept {
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

			[[nodiscard]] hybrid_value_iterator operator+(const index_type& range_) const noexcept {
				return hybrid_value_iterator { (_ptr + range_) };
			}

			[[nodiscard]] hybrid_value_iterator operator-(const index_type& range_) const noexcept {
				return hybrid_value_iterator { (_ptr - range_) };
			}

			hybrid_value_iterator& operator+=(const index_type& range_) const noexcept {
				return (_ptr += range_), *this;
			}

			hybrid_value_iterator& operator-=(const index_type& range_) const noexcept {
				return (_ptr -= range_), *this;
			}

			[[nodiscard]] bool operator==(const hybrid_value_iterator& other_) const noexcept {
				return other_._ptr == _ptr;
			}

			[[nodiscard]] bool operator!=(const hybrid_value_iterator& other_) const noexcept {
				return other_._ptr != _ptr;
			}

			[[nodiscard]] bool operator<(const hybrid_value_iterator& other_) const noexcept {
				return other_._ptr < _ptr;
			}

			[[nodiscard]] bool operator>(const hybrid_value_iterator& other_) const noexcept {
				return other_._ptr > _ptr;
			}

			[[nodiscard]] bool operator<=(const hybrid_value_iterator& other_) const noexcept {
				return other_._ptr <= _ptr;
			}

			[[nodiscard]] bool operator>=(const hybrid_value_iterator& other_) const noexcept {
				return other_._ptr >= _ptr;
			}

			hybrid_value_iterator& operator=(IN hybrid_value_iterator&& other_) noexcept {
				return (_ptr = std::exchange(other_._ptr, nullptr)), *this;
			}

			hybrid_value_iterator& operator=(const hybrid_value_iterator& other_) noexcept {
				if (this == &other_) {
					return *this;
				}
				return (_ptr = other_._ptr), *this;
			}

			const value_type& operator*() const {
				return *_ptr;
			}

			value_type& operator*() {
				return *_ptr;
			}

			const value_type* operator->() const {
				return _ptr;
			}

			value_type* operator->() {
				return _ptr;
			}

		private:
			value_type* _ptr;
		};

		/**
		 * A hybrid key value iterator. This class cannot be inherited..
		 *
		 * @author Julius
		 * @date 25.10.2020
		 */
		template <bool Const>
		class hybrid_key_value_iterator final {
		public:
			friend class hybrid_storage<KeyType, ValueType, InvalidKey>;

		public:
			using iterator_key_type = const key_type;
			using iterator_value_type = std::conditional_t<Const, const value_type, value_type>;

			/**
			 * Default constructor
			 *
			 * @author Julius
			 * @date 02.11.2020
			 */
			constexpr hybrid_key_value_iterator() noexcept:
				_keys(nullptr),
				_values(nullptr) {}

			constexpr hybrid_key_value_iterator(::std::nullptr_t) noexcept :
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
			hybrid_key_value_iterator(IN key_type* keys_, IN value_type* values_) noexcept:
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
			hybrid_key_value_iterator(IN key_type* keys_, IN value_type* values_, const index_type idx_) noexcept:
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
			hybrid_key_value_iterator(IN hybrid_key_value_iterator&& other_) noexcept:
				_keys(std::exchange(other_._keys, nullptr)),
				_values(std::exchange(other_._values, nullptr)) {}

			/**
			 * Copy Constructor
			 *
			 * @author Julius
			 * @date 02.11.2020
			 *
			 * @param  other_ The other.
			 */
			hybrid_key_value_iterator(IN const hybrid_key_value_iterator& other_) :
				_keys(other_._keys),
				_values(other_._values) {}

			hybrid_key_value_iterator& operator++() noexcept {
				return ++_keys, ++_values, *this;
			}

			hybrid_key_value_iterator& operator--() noexcept {
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

			[[nodiscard]] hybrid_key_value_iterator operator+(const index_type& range_) const noexcept {
				return hybrid_key_value_iterator { _keys + range_, _values + range_ };
			}

			[[nodiscard]] hybrid_key_value_iterator operator-(const index_type& range_) const noexcept {
				return hybrid_key_value_iterator { _keys - range_, _values - range_ };
			}

			hybrid_key_value_iterator& operator+=(const index_type& range_) noexcept {
				return (_keys += range_), (_values += range_), *this;
			}

			hybrid_key_value_iterator& operator-=(const index_type& range_) noexcept {
				return (_keys -= range_), (_values -= range_), *this;
			}

			[[nodiscard]] bool operator==(const hybrid_key_value_iterator& other_) const noexcept {
				return other_._keys == _keys && other_._values == _values;
			}

			[[nodiscard]] bool operator!=(const hybrid_key_value_iterator& other_) const noexcept {
				return other_._keys != _keys || other_._values != _values;
			}

			[[nodiscard]] bool operator<(const hybrid_key_value_iterator& other_) const noexcept {
				return other_._keys < _keys && other_._values < _values;
			}

			[[nodiscard]] bool operator>(const hybrid_key_value_iterator& other_) const noexcept {
				return other_._keys > _keys && other_._values > _values;
			}

			[[nodiscard]] bool operator<=(const hybrid_key_value_iterator& other_) const noexcept {
				return other_._keys <= _keys && other_._values <= _values;
			}

			[[nodiscard]] bool operator>=(const hybrid_key_value_iterator& other_) const noexcept {
				return other_._keys >= _keys && other_._values >= _values;
			}

			hybrid_key_value_iterator& operator=(IN hybrid_key_value_iterator&& other_) noexcept {
				_keys = std::exchange(other_._keys, nullptr);
				_values = std::exchange(other_._values, nullptr);
				return *this;
			}

			hybrid_key_value_iterator& operator=(const hybrid_key_value_iterator& other_) noexcept {
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
			[[nodiscard]] std::pair<iterator_key_type&, iterator_value_type&> operator*() const {
				return std::pair<iterator_key_type&, iterator_value_type&> {
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
			[[nodiscard]] std::pair<iterator_key_type&, iterator_value_type&> operator*() {
				return std::pair<iterator_key_type&, iterator_value_type&> {
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
			[[nodiscard]] std::pair<iterator_key_type&, iterator_value_type&> operator->() const {
				return std::pair<iterator_key_type&, iterator_value_type&> {
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
			[[nodiscard]] std::pair<iterator_key_type&, iterator_value_type&> operator->() {
				return std::pair<iterator_key_type&, iterator_value_type&> {
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
			[[nodiscard]] std::size_t diff(const this_type& other_) const {
				return MAX(_keys, other_._keys) - MIN(_keys, other_._keys);
			}

		private:
			key_type* _keys;
			value_type* _values;
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
			return iterator {
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
			return iterator {
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
			return const_iterator {
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
			return const_iterator {
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
			return const_iterator {
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
			return const_iterator {
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
			page_sequence& merge(const index_type& index_) {
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
			page_sequence& merge(const page_sequence& other_) {
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
			[[nodiscard]] bool mergable(const index_type& index_) const {
				return (end + 1u == index_) || (begin > 0u && begin - 1u == index_);
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
			[[nodiscard]] bool mergable(const page_sequence& other_) const {
				return (end + 1u == other_.begin) || (begin > 0u && begin - 1u == other_.end);
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
			[[nodiscard]] bool contains(const index_type& idx_) const {
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
			page_sequence& operator++() noexcept {
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
			page_sequence& operator--() noexcept {
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
			page_sequence& operator++(int) noexcept {
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
			page_sequence& operator--(int) noexcept {
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
			[[nodiscard]] std::pair<page_sequence, page_sequence> resect(const index_type& exclude_) const {
				return {
					{ begin, exclude_ - 1u },
					{ exclude_ + 1u, end }
				};
			}

		public:
			/**
			 * Equality Operator
			 *
			 * @author Julius
			 * @date 30.10.2021
			 *
			 * @param other_ The other page_sequence to compare to.
			 *
			 * @returns True is this and other are equal, otherwise false.
			 */
			[[nodiscard]] bool operator==(_In_ cref<page_sequence> other_) const noexcept {
				return begin == other_.begin && end == other_.end;
			}

			/**
			 * Inequality Operator
			 *
			 * @author Julius
			 * @date 30.10.2021
			 *
			 * @param other_ The other page_sequence to compare to.
			 *
			 * @returns True is this and other are not equal, otherwise false.
			 */
			[[nodiscard]] bool operator!=(_In_ cref<page_sequence> other_) const noexcept {
				return begin != other_.begin || end != other_.end;
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
			bool operator()(const page_sequence& left_, const page_sequence& right_) {
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
			_keys(static_cast<KeyType*>(static_cast<void*>(static_cast<ValueType*>(_mem) + per_page))),
			_values(static_cast<ValueType*>(_mem)),
			_seq(Vector<page_sequence>({ page_sequence { 0u, per_page - 1u } })) {}

	private:
		/**
		 * The managed and containing memory block
		 */
		void* _mem;

		key_type* _keys;// _mem[per_page * sizeof(value_type)]
		value_type* _values;// _mem[0]

		Vector<page_sequence> _seq;

		/**
		 * Pops the slot
		 *
		 * @author Julius
		 * @date 25.10.2020
		 *
		 * @returns An index_type.
		 */
		[[nodiscard]] FORCE_INLINE inline index_type pop_slot() {

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
			auto& s = _seq.back();

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
		FORCE_INLINE inline void pop_slot(const index_type& idx_) {

			/**
			 * Find sequence
			 */
			auto s = std::find_if(
				_seq.rbegin(),
				_seq.rend(),
				[&idx_](const page_sequence& entry_) {
					return entry_.contains(idx_);
				}
			);

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
				std::sort(_seq.begin(), _seq.end(), page_sequence_range_comparator());
			}
		}

		/**
		 * Inplace construct a stored type element by given index
		 *
		 * @author Julius
		 * @date 02.12.2021
		 *
		 * @param idx_ The index where to store.
		 * @param key_ The related key to store.
		 * @param args_ The packed parameter list to construct element.
		 */
		template <typename... Args_>
		FORCE_INLINE inline void place(const index_type& idx_, const key_type& key_, Args_&&... args_) {
			/**
			 *
			 */
			_keys[idx_] = key_;

			/**
			 *
			 */
			new(_values + idx_) value_type(std::forward<Args_>(args_)...);
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
		FORCE_INLINE inline void replace(
			const index_type& idx_,
			const key_type& key_,
			IN value_type&& value_
		) noexcept {
			_keys[idx_] = key_;
			destruct_inplace<value_type>(&_values[idx_]);
			construct_inplace<value_type>(&_values[idx_], std::forward<value_type>(value_));
		}

		/**
		 * Allocate memory block and guarantees empty key space
		 *
		 * @author Julius
		 * @date 23.10.2020
		 *
		 * @returns Null if it fails, else a pointer to a void.
		 */
		static void* allocate_safe() noexcept {

			void* page = malloc(page_size);

			/**
			 * 
			 */
			constexpr size_t key_length = key_size * per_page;

			auto* key_ptr = static_cast<KeyType*>(
				static_cast<void*>(
					static_cast<ValueType*>(page) + per_page
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
	 * @tparam KeyType_ Type of the key to index.
	 * @tparam StoredType_ Type of the stored type.
	 * @tparam InvalidKey_ The key value to mask an invalid key.
	 */
	template <typename KeyType_, typename StoredType_, KeyType_ InvalidKey_>
	class hybrid_storage final {
	public:
		using this_type = hybrid_storage<KeyType_, StoredType_, InvalidKey_>;

	private:
		// TODO: Hopscotch Hashing with robust set to store actor_guid with indirection_keys
		// TODO: Indexing with bitmask separation
		constexpr static uint64_t index_page_mask = 0xFFFFFFFF00000000;
		constexpr static uint64_t index_value_mask = 0x00000000FFFFFFFF;
		constexpr static uint8_t index_page_shift = 32u;

		using page_index_type = uint32_t;
		using value_index_type = uint32_t;
		using index_type = uint64_t;

		using storage_page_type = hybrid_storage_page<StoredType_, KeyType_, InvalidKey_, value_index_type>;

		/**
		 * A storage unique identifier index pair.
		 *
		 * @author Julius
		 * @date 30.10.2020
		 */
		struct storage_kv_pair {
			KeyType_ key;
			index_type idx;
		};

		/**
		 * A unique identifier index hasher.
		 *
		 * @author Julius
		 * @date 30.10.2020
		 */
		struct storage_kv_hasher :
			private std::hash<KeyType_> {
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
			[[nodiscard]] std::size_t operator()(const storage_kv_pair& value_) const {
				return static_cast<const std::hash<KeyType_>&>(*this)(value_.key);
			}
		};

		/**
		 * A unique identifier index equal.
		 *
		 * @author Julius
		 * @date 30.10.2020
		 */
		struct storage_kv_equal :
			private std::equal_to<KeyType_> {
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
			[[nodiscard]] bool operator()(
				const storage_kv_pair& left_,
				const storage_kv_pair& right_
			) const {
				return static_cast<const std::equal_to<KeyType_>&>(*this)(left_.key, right_.key);
			}
		};

		/**
		 * A unique identifier index less.
		 *
		 * @author Julius
		 * @date 30.10.2020
		 */
		struct storage_kv_less :
			private std::less<KeyType_> {
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
			[[nodiscard]] bool operator()(
				const storage_kv_pair& left_,
				const storage_kv_pair& right_
			) const {
				return static_cast<const std::less<KeyType_>&>(*this)(left_.key, right_.key);
			}
		};

	public:
		using mapping_container = std::unordered_set<storage_kv_pair, storage_kv_hasher, storage_kv_equal>;
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
		template <bool Const>
		class storage_key_value_iterator final {
		public:
			using this_type = storage_key_value_iterator<Const>;

			using key_type = KeyType_;
			using value_type = StoredType_;

			using storage_page_iterator_type = typename storage_page_type::template hybrid_key_value_iterator<Const>;

			using page_collection_type = Vector<storage_page_type>;
			using page_iterator_type = std::conditional_t<Const, typename page_collection_type::const_iterator,
				typename page_collection_type::iterator>;

		public:
			using iterator_category = std::forward_iterator_tag;

			/**
			 * Default constructor
			 *
			 * @author Julius
			 * @date 30.10.2020
			 */
			storage_key_value_iterator() noexcept = default;

			storage_key_value_iterator(page_iterator_type pageIter_) :
				_pageIterator(pageIter_),
				_pairIterator(pageIter_->begin()) {}

			storage_key_value_iterator(page_iterator_type pageIter_, storage_page_iterator_type pairIter_) :
				_pageIterator(pageIter_),
				_pairIterator(pairIter_) {}

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
			[[nodiscard]] storage_page_type& page() const {
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
			[[nodiscard]] storage_page_type& page() {
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
			[[nodiscard]] const key_type& get_key() const {
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
			[[nodiscard]] const key_type& get_key() {
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
			[[nodiscard]] const value_type& get_value() const {
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
			[[nodiscard]] const value_type& get_value() {
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
			std::pair<const key_type&, const value_type&> operator*() const {
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
			std::pair<const key_type&, value_type&> operator*() {
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
			std::pair<const key_type&, const value_type&> operator->() const {
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
			std::pair<const key_type&, value_type&> operator->() {
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
			this_type& operator++() {
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
				auto target { *this };
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
			this_type& operator+=(const index_type range_) {
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
			friend bool operator==(const this_type& left_, const this_type& right_) {
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
			friend bool operator!=(const this_type& left_, const this_type& right_) {
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
		hybrid_storage(hybrid_storage&& other_) noexcept:
			_pages(std::move(other_._pages)),
			_indirection(std::move(other_._indirection)) {}

		/**
		 * Explicitly deleted Copy Constructor
		 *
		 * @author Julius
		 * @date 25.10.2020
		 *
		 * @param  other_ The other.
		 */
		hybrid_storage(const hybrid_storage& other_) = delete;

		/**
		 * Destructor
		 *
		 * @author Julius
		 * @date 25.10.2020
		 */
		~hybrid_storage() = default;

		/**
		 * Emplaces an element with the given unique identifier and returns stable reference
		 *
		 * @author Julius
		 * @date 30.10.2020
		 *
		 * @param  key_ The key to emplace at.
		 *
		 * @returns A pair of the stored type and boolean
		 */
		std::pair<StoredType_*, bool> emplace(const KeyType_& key_) {

			std::pair<typename mapping_container::iterator, bool> er = _indirection.emplace(
				storage_kv_pair {
					key_,
					0
				}
			);
			storage_kv_pair& inp = const_cast<storage_kv_pair&>(*er.first);

			/**
			 * Check whether emplace at indirection failed
			 */
			StoredType_* ptr;
			if (er.second == false) {
				/**
				 * If emplace failed, get reference of already existing element
				 */
				ptr = &_pages[unmask_page_index(inp.idx)].get_value(unmask_value_index(inp.idx));
			} else {
				/**
				 * If emplace succeeded, construct element in pages compressed storage
				 */
				const index_type cidx = pages_insert_default(key_);

				/**
				 * Store cidx to indirection entry
				 */
				inp.idx = cidx;

				/**
				 * Get reference to existing element
				 */
				ptr = &_pages[unmask_page_index(cidx)].get_value(unmask_value_index(cidx));
			}

			return std::make_pair(ptr, er.second);
		}

		/**
		 * Emplaces a stored type relative to the given key by inplace construction
		 *
		 * @author Julius
		 * @date 02.12.2021
		 *
		 * @tparam Args_ The packed parameter type list to construct stored type.
		 *
		 * @param key_ The key where to store the element.
		 * @param args_ The packed typed parameter list to construct stored type.
		 *
		 * @returns A pair of a pointer of the stored value and a boolean
		 */
		template <typename... Args_>
		std::pair<StoredType_*, bool> emplace(const KeyType_& key_, Args_&&... args_) {

			auto er = _indirection.emplace(
				storage_kv_pair {
					key_,
					0
				}
			);

			storage_kv_pair& inp { const_cast<storage_kv_pair&>(*er.first) };

			/**
			 * Check whether emplace at indirection failed
			 */
			StoredType_* stp;
			if (er.second == false) {
				/**
				 * If emplace failed, lookup existing element and return pointer
				 */
				stp = &_pages[unmask_page_index(inp.idx)].get_value(unmask_value_index(inp.idx));
			} else {
				/**
				 * If emplace succeeded, construct element inplace at page
				 */
				const index_type cidx { pages_insert_pair<Args_...>(key_, std::forward<Args_>(args_)...) };

				/**
				 * Store cidx to indirection entry
				 */
				inp.idx = cidx;

				/**
				 * Get reference to existing element
				 */
				stp = &_pages[unmask_page_index(cidx)].get_value(unmask_value_index(cidx));
			}

			return std::make_pair(stp, er.second);
		}

		/**
		 * Inserts a component with the given unique identifier
		 *
		 * @author Julius
		 * @date 30.10.2020
		 *
		 * @param  key_ The key where to insert
		 *
		 * @returns A pair&lt;ComponentType&amp;,bool&gt;
		 */
		[[nodiscard]] std::pair<StoredType_*, bool> insert(const KeyType_& key_) {
			return emplace(key_);
		}

		/**
		 * Inserts a move constructed component with the given unique identifier
		 *
		 * @author Julius
		 * @date 30.10.2020
		 *
		 * @param 		   key_ The key where to insert.
		 * @param [in,out] value_ The component to insert.
		 *
		 * @returns A pair&lt;ComponentType&amp;,bool&gt;
		 */
		[[nodiscard]] std::pair<StoredType_*, bool> insert(const KeyType_& key_, StoredType_&& value_) {
			return emplace(key_, std::forward<StoredType_>(value_));
		}

		/**
		 * Inserts an or assign a key value pair to storage
		 *
		 * @author Julius
		 * @date 30.10.2020
		 *
		 * @param 		   key_ The key where to insert or assign.
		 * @param [in,out] value_ The value to insert or assign.
		 *
		 * @returns A reference to a ComponentType.
		 */
		StoredType_& insert_or_assign(const KeyType_& key_, StoredType_&& value_) {

			/**
			 * Don't insert_or_assign, cause we need old indirection idx from inserted element to override old value
			 *	-> This also guarantees, that component constructed by actor_guid is reference stable
			 */
			typename mapping_container::insert_or_assign_result_type ioar = _indirection.emplace(
				storage_kv_pair { key_, 0 }
			);
			storage_kv_pair& inp = ioar.first.value();

			/**
			 * Check whether emplace failed
			 */
			StoredType_* ptr;
			if (inp.second == false) {
				/**
				 * If emplace failed, replace existing value
				 */
				storage_page_type& page = _pages[unmask_page_index(inp.idx)];

				/**
				 * Replace existing value and key at storage page
				 */
				page.replace(unmask_value_index(inp.idx), key_, std::forward<StoredType_>(value_));

				/**
				 * Get reference to existing element
				 */
				ptr = &page.get_value(unmask_value_index(inp.idx));
			} else {
				/**
				 * If emplace succeeded, insert key component pair to storage page
				 */
				const index_type cidx = pages_insert_pair(key_, std::forward<StoredType_>(value_));

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
		 * @exception std::Thrown when a Standard error condition occurs.
		 *
		 * @param  key_ The key where to get the value reference from.
		 *
		 * @returns A reference to a const ComponentType.
		 */
		const StoredType_& unsafe_get(const KeyType_& key_) const {

			typename mapping_container::const_iterator cit = _indirection.find(storage_kv_pair { key_, 0 });

			if (cit == _indirection.cend()) {
				throw std::out_of_range(
					"Can not get constant reference to component, while key_ does not link to one."
				);
			}

			return _pages[unmask_page_index(cit.value().idx)].get_value(unmask_value_index(cit.value().idx));
		}

		/**
		 * Gets a constant component reference using the given unique identifier
		 *
		 * @author Julius
		 * @date 30.10.2020
		 *
		 * @exception std::Thrown when a Standard error condition occurs.
		 *
		 * @param  key_ The key where to get from.
		 * @param  hash_ The hash.
		 *
		 * @returns A reference to a const ComponentType.
		 */
		const StoredType_& unsafe_get(const KeyType_& key_, const hash_type hash_) const {

			typename mapping_container::const_iterator cit = _indirection.find(
				storage_kv_pair { key_, 0 },
				hash_
			);

			if (cit == _indirection.cend()) {
				throw std::out_of_range(
					"Can not get constant reference to component, while key_ does not link to one."
				);
			}

			return _pages[unmask_page_index(cit.value().idx)].get_value(unmask_value_index(cit.value().idx));
		}

		/**
		 * Gets a component reference using the given unique identifier
		 *
		 * @author Julius
		 * @date 30.10.2020
		 *
		 * @exception std::Thrown when a Standard error condition occurs.
		 *
		 * @param  key_ The key where to get from.
		 *
		 * @returns A reference to a ComponentType.
		 */
		StoredType_& unsafe_get(const KeyType_& key_) {

			typename mapping_container::iterator cit = _indirection.find(storage_kv_pair { key_, 0 });

			if (cit == _indirection.end()) {
				throw std::out_of_range("Can not get reference to component, while key_ does not link to one.");
			}

			return _pages[unmask_page_index(cit.value().idx)].get_value(unmask_value_index(cit.value().idx));
		}

		/**
		 * Gets a component reference using the given unique identifier
		 *
		 * @author Julius
		 * @date 30.10.2020
		 *
		 * @exception std::Thrown when a Standard error condition occurs.
		 *
		 * @param  key_ The key where to get from.
		 * @param  hash_ The hash.
		 *
		 * @returns A reference to a ComponentType.
		 */
		StoredType_& unsafe_get(const KeyType_& key_, const hash_type hash_) {

			typename mapping_container::iterator cit = _indirection.
				find(storage_kv_pair { key_, 0 }, hash_);

			if (cit == _indirection.end()) {
				throw std::out_of_range("Can not get reference to component, while key_ does not link to one.");
			}

			return _pages[unmask_page_index(cit.value().idx)].get_value(unmask_value_index(cit.value().idx));
		}

		/**
		 * Gets a constant component pointer using the given unique identifier
		 *
		 * @author Julius
		 * @date 02.11.2020
		 *
		 * @param  key_ The key where to get from.
		 *
		 * @returns Null if it fails, else a pointer to a const ComponentType.
		 */
		const StoredType_* get(const KeyType_& key_) const {

			typename mapping_container::const_iterator cit = _indirection.find(storage_kv_pair { key_, 0 });

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
		 * @param  key_ The key where to get from
		 * @param  hash_ The hash.
		 *
		 * @returns Null if it fails, else a pointer to a const ComponentType.
		 */
		const StoredType_* get(const KeyType_& key_, const hash_type hash_) const {

			typename mapping_container::const_iterator cit = _indirection.find(
				storage_kv_pair { key_, 0 },
				hash_
			);

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
		 * @param  key_ The key where to get from.
		 *
		 * @returns Null if it fails, else a pointer to a ComponentType.
		 */
		StoredType_* get(const KeyType_& key_) {

			typename mapping_container::iterator cit = _indirection.find(storage_kv_pair { key_, 0 });

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
		 * @param  key_ The key where to get from.
		 * @param  hash_ The hash.
		 *
		 * @returns Null if it fails, else a pointer to a ComponentType.
		 */
		StoredType_* get(const KeyType_& key_, const hash_type hash_) {

			typename mapping_container::iterator cit = _indirection.
				find(storage_kv_pair { key_, 0 }, hash_);

			if (cit == _indirection.end()) {
				return nullptr;
			}

			return &_pages[unmask_page_index(cit->idx)].get_value(unmask_value_index(cit->idx));
		}

		/**
		 * Erases component identified by given key_
		 *
		 * @author Julius
		 * @date 30.10.2020
		 *
		 * @exception std::Thrown when a Standard error condition occurs.
		 *
		 * @param  key_ The key where to erase.
		 */
		void erase(const KeyType_& key_) {
			typename mapping_container::const_iterator cit = _indirection.find(storage_kv_pair { key_, 0 });

			// TODO: check whether we should just return without mutation
			// TODO: if immutable check, could optimize for concurrent locking, cause existing check is read only, and does not require to lock whole structure

			/**
			 * If key_ does not exist, catch error case
			 */
			if (cit == _indirection.end()) {
				throw std::out_of_range("Can not erase element from storage, while key does not link to one");
			}

			/**
			 * Get mutation reference to storage page
			 */
			storage_page_type& page = _pages[unmask_page_index(cit->idx)];

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
			auto pb = _pages.cbegin();

			constexpr auto null_iter = typename iterator::storage_page_iterator_type {};
			return iterator {
				pb,
				pb._Unwrapped() != nullptr ? pb->cbegin() : null_iter
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

			constexpr auto null_iter = typename iterator::storage_page_iterator_type {};
			auto pair_iter = pe._Unwrapped() != nullptr ? (--pe)->end() : null_iter;
			return iterator { pe, std::move(pair_iter) };
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
			auto pb = _pages.cbegin();

			constexpr auto null_iter = typename const_iterator::storage_page_iterator_type {};
			return const_iterator {
				pb,
				pb._Unwrapped() != nullptr ? pb->cbegin() : null_iter
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

			constexpr auto null_iter = typename const_iterator::storage_page_iterator_type {};
			auto pair_iter = pe._Unwrapped() != nullptr ? (--pe)->end() : null_iter;
			return const_iterator { pe, std::move(pair_iter) };
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
			auto pb = _pages.cbegin();

			constexpr auto null_iter = typename const_iterator::storage_page_iterator_type {};
			return const_iterator {
				pb,
				pb._Unwrapped() != nullptr ? pb->cbegin() : null_iter
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

			constexpr auto null_iter = typename const_iterator::storage_page_iterator_type {};
			auto pair_iter = pe._Unwrapped() != nullptr ? (--pe)->end() : null_iter;
			return const_iterator { pe, std::move(pair_iter) };
		}

	private:
		Vector<storage_page_type> _pages;

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
		FORCE_INLINE static inline page_index_type unmask_page_index(const uint64_t masked_) {
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
		FORCE_INLINE static inline value_index_type unmask_value_index(const uint64_t masked_) {
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
		FORCE_INLINE static inline void unmask(IN const uint64_t masked_, OUT uint64_t& page_, OUT uint64_t& value_) {
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
		const mapping_container& keys() const noexcept {
			return _indirection;
		}

	private:
		/**
		 * Insert key value pair to storage pages
		 *
		 * @author Julius
		 * @date 30.10.2020
		 *
		 * @param  key_ The indexing key.
		 * @param  value_ The indexed value.
		 *
		 * @returns An index_type.
		 */
		[[nodiscard]] index_type pages_insert_pair(const KeyType_& key_, const StoredType_& value_) {

			// TODO: optimize way to find storage page with empty sequence
			// TODO: optimize way reusage of pages / using reverse iterator will speed up linear insert, but slow down on reusage
			auto s = std::find_if(
				_pages.rbegin(),
				_pages.rend(),
				[](const storage_page_type& page_) {
					return page_.can_store();
				}
			);

			/**
			 * Check whether there is a page with empty space to store data
			 */
			if (s == _pages.rend()) {

				/**
				 * If no space found, create new page and try again
				 */
				_pages.push_back(storage_page_type {});

				/**
				 * Recursive try again
				 */
				return pages_insert_pair(key_, value_);
			}

			/**
			 * Get mutation reference to storage page
			 */
			storage_page_type& page = *s;

			/**
			 * Emplace data to page
			 */
			index_type vidx;
			[[maybe_unused]] const bool success = page.emplace(key_, value_, vidx);

			const index_type pidx = std::distance(_pages.begin(), s.base()) - 1;

			/**
			 * Composite index_type
			 */
			return (vidx | (pidx << index_page_shift));
		}

		[[nodiscard]] index_type pages_insert_default(const KeyType_& key_) {

			// TODO: optimize way to find storage page with empty sequence
			// TODO: optimize way reusage of pages / using reverse iterator will speed up linear insert, but slow down on reusage
			auto s = std::find_if(
				_pages.rbegin(),
				_pages.rend(),
				[](const storage_page_type& page_) {
					return page_.can_store();
				}
			);

			/**
			 * Check whether there is a page with empty space to store data
			 */
			if (s == _pages.rend()) {

				/**
				 * If no space found, create new page and try again
				 */
				_pages.push_back(storage_page_type {});

				/**
				 * Recursive try again
				 */
				return pages_insert_default(key_);
			}

			/**
			 * Get mutation reference to storage page
			 */
			storage_page_type& page = *s;

			/**
			 * Emplace data to page
			 */
			value_index_type vidx;
			[[maybe_unused]] const bool success = page.template emplace<>(key_, vidx);

			const page_index_type pidx = std::distance(_pages.begin(), s.base()) - 1;

			/**
			 * Composite index_type
			 */
			return (index_type { vidx } | (index_type { pidx } << index_page_shift));
		}

		template <typename... Args_>
		[[nodiscard]] index_type pages_insert_pair(const KeyType_& key_, Args_&&... args_) {

			// TODO: optimize way to find storage page with empty sequence
			// TODO: optimize way reusage of pages / using reverse iterator will speed up linear insert, but slow down on reusage
			auto s = std::find_if(
				_pages.rbegin(),
				_pages.rend(),
				[](const storage_page_type& page_) {
					return page_.can_store();
				}
			);

			/**
			 * Check whether there is a page with empty space to store data
			 */
			if (s == _pages.rend()) {

				/**
				 * If no space found, create new page and try again
				 */
				_pages.push_back(storage_page_type {});

				/**
				 * Recursive try again
				 */
				return pages_insert_pair(key_, std::forward<Args_>(args_)...);
			}

			/**
			 * Get mutation reference to storage page
			 */
			storage_page_type& page = *s;

			/**
			 * Emplace data to page
			 */
			value_index_type vidx;
			[[maybe_unused]] const bool r = page.template emplace<Args_...>(key_, std::forward<Args_>(args_)..., vidx);

			const page_index_type pidx { static_cast<page_index_type>(std::distance(_pages.begin(), s.base()) - 1) };

			/**
			 * Composite index_type
			 */
			return (index_type { vidx } | (index_type { pidx } << index_page_shift));
		}
	};
}
