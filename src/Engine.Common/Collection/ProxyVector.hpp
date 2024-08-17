#pragma once

#include <algorithm>
#include <cstdlib>
#include <ranges>
#include <utility>

#include "../Wrapper.hpp"
#include "../__macro.hpp"
#include "../Math/Helper.hpp"
#include "../Memory/Address.hpp"
#include "../Memory/Destroy.hpp"

namespace hg {
	namespace proxyvector {
		/**
		 * An exponent 2 growth policy.
		 *
		 * @author Julius
		 * @date 27.08.2021
		 *
		 * @tparam SizeType_ Type of the size type.
		 * @tparam GrowthFactor Type of the growth factor.
		 */
		template <typename SizeType_ = std::size_t, SizeType_ GrowthFactor = 2uLL>
		struct exp2_growth_policy {
			using size_type = SizeType_;

			explicit constexpr exp2_growth_policy(const size_type min_) :
				_mask(math::fast_ceil2<SizeType_> {}(min_) - 1u) {}

			[[nodiscard]] constexpr size_type next() const {
				return (_mask + 1) * GrowthFactor;
			}

			void clear() noexcept {
				_mask = 0;
			}

			static_assert(math::fast_ceil2<SizeType_> {}.ip2(GrowthFactor), "GrowthFactor have to be a power of two");
			static_assert(GrowthFactor >= 2uLL, "GrowthFactor have to be greater or equal to 2");

		private:
			size_type _mask;
		};
	}

	template <typename Type_>
	concept IsProxyVectorType =
		std::is_nothrow_destructible_v<Type_> &&
		(std::is_nothrow_move_constructible_v<Type_>) &&
		(std::is_nothrow_move_assignable_v<Type_>);

	/**
	 * Forward Declaration
	 */
	template <
		typename Type_,
		typename SizeType_ = std::size_t,
		typename GrowthPolicy_ = proxyvector::exp2_growth_policy<>,
		bool Stable_ = false,
		bool Realloc_ = true
	>
	class TypedProxyVector;

	template <
		typename SizeType_ = std::size_t,
		typename GrowthPolicy_ = proxyvector::exp2_growth_policy<>,
		bool Stable_ = false,
		bool Realloc_ = true
	>
	class ProxyVector {
	public:
		using size_type = SizeType_;
		using count_type = SizeType_;
		using growth_policy = GrowthPolicy_;

		using value_type = ProxyVector<size_type, growth_policy, Stable_, Realloc_>;
		using reference_type = ref<value_type>;
		using const_reference_type = cref<value_type>;

	public:
		using proxy_holder_type = struct proxy_holder_type {
			using align_size_type = size_t;
			align_size_type align;

			using destructor_type = void (*)(ptr<void> this_) noexcept;
			destructor_type destructor;

			using copy_constructor_type = void (*)(ptr<void> this_, ptr<void> other_);
			copy_constructor_type copy_constructor;

			using move_constructor_type = void (*)(ptr<void> this_, ptr<void> other_) noexcept;
			move_constructor_type move_constructor;

			using copy_assignment_type = void (*)(ptr<void> this_, ptr<void> other_);
			copy_assignment_type copy_assignment;

			using move_assignment_type = void (*)(ptr<void> this_, ptr<void> other_) noexcept;
			move_assignment_type move_assignment;
		};

	private:
		proxy_holder_type _proxy;

	public:
		ProxyVector() = delete;

		/**
		 * Move Constructor
		 *
		 * @author Julius
		 * @date 27.08.2021
		 *
		 * @param  other_ The other.
		 */
		ProxyVector(mref<value_type> other_) noexcept :
			_proxy(std::move(other_._proxy)),
			_base(std::exchange(other_._base, nullptr)),
			_last(std::exchange(other_._last, nullptr)),
			_end(std::exchange(other_._end, nullptr)) {}

		/**
		 * Copy Constructor
		 *
		 * @author Julius
		 * @date 27.08.2021
		 *
		 * @param  other_ The other.
		 */
		ProxyVector(const_reference_type other_) :
			_proxy(other_._proxy),
			_base(nullptr),
			_last(nullptr),
			_end(nullptr) {
			copy_assign(other_);
		}

	private:
		constexpr ProxyVector(cref<proxy_holder_type> proxy_) :
			_proxy(proxy_),
			_base(nullptr),
			_last(nullptr),
			_end(nullptr) {}

	public:
		/**
		 * Creates a instance of ProxyVector<...> with provided proxy_
		 *
		 * @author Julius
		 * @date 27.08.2021
		 *
		 * @param  proxy_ The proxy.
		 *
		 * @returns The ProxyVector<...> instance.
		 */
		[[nodiscard]] constexpr static value_type make(proxy_holder_type proxy_) {
			return value_type { proxy_ };
		}

	private:
		template <typename Type_, typename First_>
		constexpr void apply_unfolded_make(mref<First_> first_) {
			emplace_back(std::forward<Type_>(first_));
		}

		template <typename Type_, typename First_, typename... Args_>
		constexpr void apply_unfolded_make(mref<First_> first_, mref<Args_>... args_) {
			emplace_back(std::forward<Type_>(first_));
			apply_unfolded_make<Type_, Args_...>(std::forward<Args_>(args_)...);
		}

	public:
		/**
		 * Creates a instance of ProxyVector<..> with provided proxy_
		 *  and pushed vardiac parameters as elements
		 *
		 * @tparam Type_ Type of the type.
		 * @tparam Args_ Type of the arguments.
		 * @param  proxy_ The proxy.
		 * @param  args_ The args.
		 *
		 * @returns The ProxyVector<...> instance.
		 */
		template <typename Type_, typename... Args_>
		[[nodiscard]] constexpr static value_type make(cref<proxy_holder_type> proxy_, Args_... args_) {
			auto self { make(proxy_) };
			self.reserve(sizeof...(Args_));
			self.template apply_unfolded_make<Type_, Args_...>(std::forward<Args_>(args_)...);
			return self;
		}

	public:
		/**
		 * Destructor
		 *
		 * @author Julius
		 * @date 27.08.2021
		 */
		~ProxyVector() noexcept {
			tidy();
		}

	private:
		void tidy() noexcept {
			if (_base) {
				destroy_range(_base, _last);
				std::free(_base);

				_base = nullptr;
				_last = nullptr;
				_end = nullptr;
			}
		}

		template <typename Type_>
		void tidy() noexcept {
			if (_base) {
				destroy_range<Type_>(_base, _last);

				using allocator_type = std::allocator<Type_>;
				using allocator_trait = std::allocator_traits<allocator_type>;

				allocator_type alloc {};
				alloc.deallocate(
					_base,
					static_cast<size_t>(
						static_cast<ptr<Type_>>(_end) - static_cast<ptr<Type_>>(_base)
					)
				);

				_base = nullptr;
				_last = nullptr;
				_end = nullptr;
			}
		}

	private:
		ptr<void> _base;
		ptr<void> _last;
		ptr<void> _end;

	public:
		/**
		 * Gets the size
		 *
		 * @author Julius
		 * @date 27.08.2021
		 *
		 * @returns The stored element count.
		 */
		[[nodiscard]] count_type size() const noexcept {
			return static_cast<count_type>(
				static_cast<ptr<char>>(_last) - static_cast<ptr<char>>(_base)
			) / _proxy.align;
		}

		/**
		 * Gets the size
		 *
		 * @tparam Type_ Type of the type.
		 *
		 * @returns The stored element count.
		 */
		template <typename Type_>
		[[nodiscard]] count_type size() const noexcept {
			return std::ranges::distance(
				static_cast<ptr<Type_>>(_base),
				static_cast<ptr<Type_>>(_last)
			);
		}

		/**
		 * Gets the capacity
		 *
		 * @author Julius
		 * @date 27.08.2021
		 *
		 * @returns The internal capacity for elements.
		 */
		[[nodiscard]] count_type capacity() const noexcept {
			return static_cast<count_type>(
				static_cast<ptr<char>>(_end) - static_cast<ptr<char>>(_base)
			) / _proxy.align;
		}

		/**
		 * Gets the capacity
		 *
		 * @tparam Type_ Type of the type.
		 *
		 * @returns The internal capacity for elements.
		 */
		template <typename Type_>
		[[nodiscard]] count_type capacity() const noexcept {
			return std::ranges::distance(
				static_cast<ptr<Type_>>(_base),
				static_cast<ptr<Type_>>(_end)
			);
		}

		/**
		 * Check whether this is empty
		 *
		 * @author Julius
		 * @date 27.08.2021
		 *
		 * @returns True if it succeeds, false if it fails.
		 */
		[[nodiscard]] bool empty() const noexcept {
			return _last == _base;
		}

	private:
		template <typename Type_>
		class ProxyVectorIterator {
		private:
			friend class ProxyVector;

		public:
			using this_type = ProxyVectorIterator<Type_>;
			using reference_this_type = ref<this_type>;
			using const_reference_this_type = cref<this_type>;

			using value_type = Type_;
			using reference_type = ref<value_type>;
			using const_reference_type = cref<value_type>;
			using pointer_type = ptr<value_type>;

			// Iterator Traits
			using difference_type = std::ptrdiff_t;
			using pointer = pointer_type;
			using reference = reference_type;

			#ifdef __cpp_lib_concepts
			using iterator_concept = std::contiguous_iterator_tag;
			#endif

			using iterator_category = std::random_access_iterator_tag;

		public:
			ProxyVectorIterator() = delete;

			ProxyVectorIterator(const_reference_this_type other_) noexcept :
				_cur(other_._cur) {}

			ProxyVectorIterator(this_type&& other_) noexcept :
				_cur(std::move(other_._cur)) {}

			ProxyVectorIterator(pointer_type pos_) noexcept :
				_cur(pos_) {}

			~ProxyVectorIterator() noexcept = default;

		public:
			reference_this_type operator=(const_reference_this_type other_) noexcept {
				return _cur = other_._cur, *this;
			}

			reference_this_type operator=(this_type&& other_) noexcept {
				if (this != &other_) {
					_cur = std::move(other_._cur);
				}
				return *this;
			}

		private:
			pointer_type _cur;

		public:
			[[nodiscard]] const_reference_type operator*() const noexcept {
				return *_cur;
			}

			[[nodiscard]] reference_type operator*() noexcept {
				return *_cur;
			}

			[[nodiscard]] pointer_type operator->() const noexcept {
				return _cur;
			}

			[[nodiscard]] const_reference_this_type operator[](size_type offset_) const noexcept {
				return _cur[offset_];
			}

			[[nodiscard]] reference_this_type operator[](size_type offset_) noexcept {
				return _cur[offset_];
			}

		public:
			reference_this_type operator++() noexcept {
				return ++_cur, *this;
			}

			this_type operator++(int) noexcept {
				auto tmp { *this };
				++_cur;
				return tmp;
			}

			reference_this_type operator--() noexcept {
				return --_cur, *this;
			}

			this_type operator--(int) noexcept {
				auto tmp { *this };
				--_cur;
				return tmp;
			}

		public:
			reference_this_type operator+=(size_type distance_) noexcept {
				return _cur += distance_, *this;
			}

			reference_this_type operator-=(size_type distance_) noexcept {
				return _cur -= distance_, *this;
			}

		public:
			[[nodiscard]] this_type operator+(size_type distance_) const noexcept {
				auto tmp { *this };
				return tmp += distance_, tmp;
			}

			[[nodiscard]] this_type operator+(size_type distance_) noexcept {
				auto tmp { *this };
				return tmp += distance_, tmp;
			}

			[[nodiscard]] this_type operator-(size_type distance_) const noexcept {
				auto tmp { *this };
				return tmp -= distance_, tmp;
			}

			[[nodiscard]] this_type operator-(size_type distance_) noexcept {
				auto tmp { *this };
				return tmp -= distance_, tmp;
			}

		public:
			[[nodiscard]] bool operator==(const_reference_this_type other_) const noexcept {
				return _cur == other_._cur;
			}

			[[nodiscard]] bool operator!=(const_reference_this_type other_) const noexcept {
				return _cur != other_._cur;
			}

			[[nodiscard]] bool operator<(const_reference_this_type other_) const noexcept {
				return _cur < other_._cur;
			}

			[[nodiscard]] bool operator>(const_reference_this_type other_) const noexcept {
				return _cur > other_._cur;
			}

			[[nodiscard]] bool operator<=(const_reference_this_type other_) const noexcept {
				return _cur <= other_._cur;
			}

			[[nodiscard]] bool operator>=(const_reference_this_type other_) const noexcept {
				return _cur >= other_._cur;
			}

			[[nodiscard]] std::strong_ordering operator<=>(const_reference_this_type other_) const noexcept {
				return unfancy(_cur) <=> unfancy(other_._cur);
			}
		};

	public:
		template <typename Type_>
		using iterator_type = ProxyVectorIterator<Type_>;

		template <typename Type_>
		using const_iterator_type = const ProxyVectorIterator<Type_>;

	public:
		/**
		 * Get an iterator to the first element
		 *
		 * @tparam Type_ Type of the type.
		 *
		 * @returns An iterator_type&lt;Type_&gt;
		 */
		template <typename Type_>
		[[nodiscard]] iterator_type<Type_> begin() noexcept {
			return { static_cast<typename iterator_type<Type_>::pointer_type>(unfancy(_base)) };
		}

		/**
		 * Get an iterator to the first element
		 *
		 * @tparam Type_ Type of the type.
		 *
		 * @returns A const_iterator_type&lt;Type_&gt;
		 */
		template <typename Type_>
		[[nodiscard]] const_iterator_type<Type_> begin() const noexcept {
			return { static_cast<typename const_iterator_type<Type_>::pointer_type>(unfancy(_base)) };
		}

		/**
		 * Get an const_iterator to the first element
		 *
		 * @tparam Type_ Type of the type.
		 *
		 * @returns A const_iterator_type&lt;Type_&gt;
		 */
		template <typename Type_>
		[[nodiscard]] const_iterator_type<Type_> cbegin() noexcept {
			return { static_cast<typename const_iterator_type<Type_>::pointer_type>(unfancy(_base)) };
		}

		/**
		 * Get an iterator behind the last element
		 *
		 * @tparam Type_ Type of the type.
		 *
		 * @returns An iterator_type&lt;Type_&gt;
		 */
		template <typename Type_>
		[[nodiscard]] iterator_type<Type_> end() noexcept {
			return { static_cast<typename iterator_type<Type_>::pointer_type>(unfancy(_last)) };
		}

		/**
		 * Get an iterator behind the last element
		 *
		 * @tparam Type_ Type of the type.
		 *
		 * @returns A const_iterator_type&lt;Type_&gt;
		 */
		template <typename Type_>
		[[nodiscard]] const_iterator_type<Type_> end() const noexcept {
			return { static_cast<typename const_iterator_type<Type_>::pointer_type>(unfancy(_last)) };
		}

		/**
		 * Get an const_iterator behind the last element
		 *
		 * @tparam Type_ Type of the type.
		 *
		 * @returns A const_iterator_type&lt;Type_&gt;
		 */
		template <typename Type_>
		[[nodiscard]] const_iterator_type<Type_> cend() const noexcept {
			return { static_cast<typename const_iterator_type<Type_>::pointer_type>(unfancy(_last)) };
		}

	public:
		/**
		 * Lookup the first element
		 *
		 * @tparam Type_ Type of the type.
		 *
		 * @returns A const reference to the first element
		 */
		template <typename Type_>
		[[nodiscard]] cref<Type_> front() const noexcept {
			return *static_cast<ptr<Type_>>(_base);
		}

		/**
		 * Lookup the first element
		 *
		 * @tparam Type_ Type of the type.
		 *
		 * @returns A reference to the first element
		 */
		template <typename Type_>
		[[nodiscard]] ref<Type_> front() noexcept {
			return *static_cast<ptr<Type_>>(_base);
		}

		/**
		 * Lookup the first element
		 *
		 * @author Julius
		 * @date 27.08.2021
		 *
		 * @returns A pointer to the first stored element
		 */
		[[nodiscard]] ptr<void> front() const noexcept {
			return _base;
		}

		/**
		 * Lookup the last element
		 *
		 * @tparam Type_ Type of the type.
		 *
		 * @returns A const reference to the last element
		 */
		template <typename Type_>
		[[nodiscard]] cref<Type_> back() const noexcept {
			return *static_cast<ptr<Type_>>(_last);
		}

		/**
		 * Lookup the last element
		 *
		 * @tparam Type_ Type of the type.
		 *
		 * @returns A reference to the last element
		 */
		template <typename Type_>
		[[nodiscard]] ref<Type_> back() noexcept {
			return *static_cast<ptr<Type_>>(_last);
		}

		/**
		 * Lookup the last element
		 *
		 * @author Julius
		 * @date 27.08.2021
		 *
		 * @returns A pointer to the last stored element
		 */
		[[nodiscard]] ptr<void> back() const noexcept {
			return _last;
		}

	private:
		template <typename Type_, typename SizeType_, typename GrowthPolicy_, bool Stable_, bool Realloc_>
		friend class TypedProxyVector;

		template <typename Type_>
		using typed_wrapper_type = TypedProxyVector<Type_, SizeType_, GrowthPolicy_, Stable_, Realloc_>;

		/**
		 * Warning: using references is fine, but using copied or moved value_type will implicitly invalidate origin container
		 */
	public:
		template <typename Type_>
		[[nodiscard]] cref<typed_wrapper_type<Type_>> typed() const noexcept {
			return *static_cast<ptr<const typed_wrapper_type<Type_>>>(this);
		}

		template <typename Type_>
		[[nodiscard]] ref<typed_wrapper_type<Type_>> typed() noexcept {
			return *static_cast<ptr<typed_wrapper_type<Type_>>>(this);
		}

	private:
		void push_back(mref<ptr<void>> value_) noexcept {
			emplace_back(std::forward<ptr<void>>(value_));
		}

	public:
		/**
		 * Will append the given value_ to this
		 *
		 * @tparam Type_ Type of the type.
		 * @param  value_ The value.
		 */
		template <typename Type_> requires std::is_move_constructible_v<Type_>
		void push_back(mref<Type_> value_) noexcept (std::is_nothrow_move_constructible_v<Type_>) {
			[[maybe_unused]] auto _ = emplace_back<Type_>(std::forward<Type_>(value_));
		}

	private:
		void push_back(cref<ptr<void>> value_) {
			emplace_back(value_);
		}

	public:
		/**
		 * Will append the given value_ to this
		 *
		 * @tparam Type_ Type of the type.
		 * @param  value_ The value.
		 */
		template <typename Type_> requires std::is_copy_constructible_v<Type_>
		void push_back(cref<Type_> value_) noexcept (std::is_nothrow_copy_constructible_v<Type_>) {
			[[maybe_unused]] auto _ = emplace_back<Type_>(value_);
		}

	private:
		constexpr size_type calculate_grow(size_type newSize_) noexcept {
			return growth_policy { newSize_ }.next();
		}

		void destroy_range(ptr<void> from_, ptr<void> to_) {

			/**
			 * Guard for non existing / required destructor
			 */
			if (!_proxy.destructor) {
				return;
			}

			using pointer_type = decltype(_base);

			ptr<char> fst { static_cast<ptr<char>>(from_) };
			const pointer_type& lst { to_ };

			for (; fst != lst; fst += _proxy.align) {
				_proxy.destructor(static_cast<pointer_type>(fst));
			}
		}

		template <typename Type_>
		void destroy_range(ptr<Type_> from_, ptr<Type_> to_) {

			using pointer_type = ptr<Type_>;

			cref<pointer_type> fst { static_cast<pointer_type>(from_) };
			cref<pointer_type> lst { static_cast<pointer_type>(to_) };

			using allocator_type = std::allocator<Type_>;

			allocator_type alloc {};
			::hg::destroy_range(fst, lst, alloc);
		}

		void change_internal(ptr<void> newFst_, const count_type newSize_, const count_type newCapacity_) {

			using pointer_type = decltype(_base);
			cref<pointer_type> myFst { _base };
			cref<pointer_type> myLst { _last };

			if (myFst) {
				destroy_range(myFst, myLst);
				std::free(_base);
			}

			_base = newFst_;
			_last = static_cast<ptr<void>>(static_cast<ptr<char>>(newFst_) + (newSize_ * _proxy.align));
			_end = static_cast<ptr<void>>(static_cast<ptr<char>>(newFst_) + (newCapacity_ * _proxy.align));
		}

		void change_internal(
			ptr<void> newFst_,
			const count_type newSize_,
			const count_type newCapacity_,
			[[maybe_unused]] std::false_type
		) {
			_base = newFst_;
			_last = static_cast<ptr<void>>(static_cast<ptr<char>>(newFst_) + (newSize_ * _proxy.align));
			_end = static_cast<ptr<void>>(static_cast<ptr<char>>(newFst_) + (newCapacity_ * _proxy.align));
		}

		template <typename Type_>
		void change_internal(ptr<Type_> newFst_, const count_type newSize_, const count_type newCapacity_) {

			using pointer_type = ptr<Type_>;
			cref<pointer_type> myFst { static_cast<pointer_type>(_base) };
			cref<pointer_type> myLst { static_cast<pointer_type>(_last) };
			cref<pointer_type> myEnd { static_cast<pointer_type>(_end) };

			using allocator_type = std::allocator<Type_>;

			allocator_type alloc {};
			if (myFst) {
				destroy_range<Type_>(myFst, myLst);
				alloc.deallocate(myFst, static_cast<size_type>(myEnd - myFst));
			}

			_base = newFst_;
			_last = newFst_ + newSize_;
			_end = newFst_ + newCapacity_;
		}

		template <typename Type_>
		void change_internal(
			ptr<Type_> newFst_,
			const count_type newSize_,
			const count_type newCapacity_,
			[[maybe_unused]] std::false_type
		) {
			_base = newFst_;
			_last = newFst_ + newSize_;
			_end = newFst_ + newCapacity_;
		}

		void emplace_back_unused(mref<ptr<void>> value_) noexcept {

			using pointer_type = decltype(_base);
			cref<pointer_type> myLst { static_cast<ptr<char>>(_last) };

			_proxy.move_constructor(myLst, value_);
			_last = static_cast<ptr<char>>(myLst) + _proxy.align;
		}

		template <typename Type_>
		iterator_type<Type_> emplace_back_unused(mref<Type_> value_) noexcept {

			#ifdef _DEBUG
			assert(sizeof(Type_) == _proxy.align);
			#endif

			using pointer_type = ptr<Type_>;
			cref<pointer_type> myLst { static_cast<pointer_type>(_last) };

			using allocator_type = std::allocator<Type_>;
			using allocator_trait = std::allocator_traits<allocator_type>;

			allocator_type alloc {};
			allocator_trait::construct(alloc, myLst, std::forward<Type_>(value_));

			pointer_type result = myLst;
			_last = myLst + 1;

			return { result };
		}

		void realloc_base_eb(
			const count_type oldSize_,
			const count_type newSize_,
			const count_type newCapacity_,
			mref<ptr<void>> value_
		) noexcept {

			using pointer_type = decltype(_base);

			pointer_type newFst, reallocBase;
			newFst = reallocBase = std::realloc(_base, newCapacity_ * _proxy.align);

			if (reallocBase == nullptr) {
				newFst = std::malloc(newCapacity_ * _proxy.align);

				ptr<char> myFst { static_cast<ptr<char>>(_base) };
				cref<pointer_type> myLst { static_cast<pointer_type>(_last) };

				ptr<char> next { static_cast<ptr<char>>(newFst) };
				for (; myFst != myLst; myFst += _proxy.align, next += _proxy.align) {
					_proxy.move_constructor(static_cast<ptr<void>>(next), static_cast<ptr<void>>(myFst));
				}
			}

			const pointer_type newLst = static_cast<ptr<char>>(newFst) + oldSize_ * _proxy.align;
			_proxy.move_constructor(newLst, value_);

			if (reallocBase != newFst && _base) {
				destroy_range(_base, _last);
				std::free(_base);
			}

			constexpr std::false_type _no_auto_dealloc_ {};
			change_internal(newFst, newSize_, newCapacity_, _no_auto_dealloc_);
		}

		void alloc_base_eb(
			const count_type oldSize_,
			const count_type newSize_,
			const count_type newCapacity_,
			mref<ptr<void>> value_
		) noexcept {

			using pointer_type = decltype(_base);

			const pointer_type newFst = std::malloc(newCapacity_ * _proxy.align);
			const pointer_type newLst = static_cast<ptr<char>>(newFst) + oldSize_ * _proxy.align;

			ptr<char> myFst { static_cast<ptr<char>>(_base) };
			cref<pointer_type> myLst { static_cast<pointer_type>(_last) };

			ptr<char> next { static_cast<ptr<char>>(newFst) };
			for (; myFst != myLst; myFst += _proxy.align, next += _proxy.align) {
				_proxy.move_constructor(static_cast<ptr<void>>(next), static_cast<ptr<void>>(myFst));
			}
			_proxy.move_constructor(newLst, value_);

			change_internal(newFst, newSize_, newCapacity_);
		}

		void emplace_back_reallocate(mref<ptr<void>> value_) noexcept {

			const auto oldSize = size();

			const count_type newSize = oldSize + 1;
			const count_type newCapacity = calculate_grow(newSize);

			if constexpr (Realloc_) {
				realloc_base_eb(oldSize, newSize, newCapacity, std::forward<ptr<void>>(value_));

			} else {
				alloc_base_eb(oldSize, newSize, newCapacity, std::forward<ptr<void>>(value_));
			}
		}

		template <typename Type_>
		iterator_type<Type_> emplace_back_reallocate(mref<Type_> value_) noexcept {

			#ifdef _DEBUG
			assert(sizeof(Type_) == _proxy.align);
			#endif

			using pointer_type = ptr<Type_>;

			cref<pointer_type> myFst { static_cast<pointer_type>(_base) };
			cref<pointer_type> myLst { static_cast<pointer_type>(_last) };

			const auto oldSize = size();

			const count_type newSize = oldSize + 1;
			const count_type newCapacity = calculate_grow(newSize);

			using allocator_type = std::allocator<Type_>;
			using allocator_trait = std::allocator_traits<allocator_type>;

			allocator_type alloc {};
			const pointer_type newFst = alloc.allocate(newCapacity);
			const pointer_type newLst = newFst + oldSize;

			std::uninitialized_move(myFst, myLst, newFst/* TODO: , alloc*/);
			allocator_trait::construct(alloc, newLst, std::forward<Type_>(value_));

			change_internal<Type_>(newFst, newSize, newCapacity);
			return { newLst };
		}

	private:
		void emplace_back(mref<ptr<void>> value_) noexcept {

			using pointer_type = decltype(_base);
			const pointer_type& myLst { _last };
			const pointer_type& myEnd { _end };

			if (myLst != myEnd) {
				emplace_back_unused(std::forward<ptr<void>>(value_));
				return;
			}

			emplace_back_reallocate(std::forward<ptr<void>>(value_));
		}

	public:
		/**
		 * Will append the given value_ to this and return iterator it
		 *
		 * @tparam Type_ Type of the type.
		 * @param  value_ The value.
		 *
		 * @returns An iterator to the stored value_
		 */
		template <typename Type_> requires std::is_move_constructible_v<Type_>
		iterator_type<Type_> emplace_back(mref<Type_> value_) noexcept (std::is_nothrow_move_constructible_v<Type_>) {

			using pointer_type = ptr<Type_>;
			const pointer_type& myLst { static_cast<pointer_type>(_last) };
			const pointer_type& myEnd { static_cast<pointer_type>(_end) };

			if (myLst != myEnd) {
				return emplace_back_unused<Type_>(std::forward<Type_>(value_));
			}

			return emplace_back_reallocate<Type_>(std::forward<Type_>(value_));
		}

		void emplace_back(cref<ptr<void>> value_) {

			using pointer_type = decltype(_base);
			cref<pointer_type> myLst { _last };
			cref<pointer_type> myEnd { _end };

			if (myLst != myEnd) {
				emplace_back_unused(std::forward<ptr<void>>(value_));
				return;
			}

			emplace_back_reallocate(std::forward<ptr<void>>(value_));
		}

		/**
		 * Will append the given value_ to this and return iterator it
		 *
		 * @tparam Type_ Type of the type.
		 * @param  value_ The value.
		 *
		 * @returns An iterator to the stored value_
		 */
		template <typename Type_> requires std::is_copy_constructible_v<Type_>
		iterator_type<Type_> emplace_back(cref<Type_> value_) noexcept (std::is_nothrow_copy_constructible_v<Type_>) {

			using pointer_type = ptr<Type_>;
			cref<pointer_type> myLst { static_cast<pointer_type>(_last) };
			cref<pointer_type> myEnd { static_cast<pointer_type>(_end) };

			if (myLst != myEnd) {
				return emplace_back_unused<Type_>(std::forward<Type_>(value_));
			}

			return emplace_back_reallocate<Type_>(std::forward<Type_>(value_));
		}

	public:
		template <typename Type_> requires std::is_move_constructible_v<Type_> && std::is_move_assignable_v<Type_>
		iterator_type<Type_> insert_or_assign(
			const_iterator_type<Type_> where_,
			mref<Type_> value_
		) noexcept (std::is_nothrow_move_constructible_v<Type_> && std::is_nothrow_move_assignable_v<Type_>) {}

		template <typename Type_> requires std::is_copy_constructible_v<Type_> && std::is_copy_assignable_v<Type_>
		iterator_type<Type_> insert_or_assign(
			const_iterator_type<Type_> where_,
			cref<Type_> value_
		) noexcept (std::is_nothrow_copy_constructible_v<Type_> && std::is_nothrow_copy_assignable_v<Type_>) {}

	public:
		/**
		 * Will remove the last stored element
		 *
		 * @author Julius
		 * @date 27.08.2021
		 */
		void pop_back() noexcept {

			using pointer_type = decltype(_base);
			cref<pointer_type> myLst { static_cast<pointer_type>(_last) };

			_proxy.destructor(myLst);
			_last = static_cast<ptr<char>>(myLst) - _proxy.align;
		}

		/**
		 * Will remove the last stored element
		 *
		 * @tparam Type_ Type of the type.
		 */
		template <typename Type_>
		void pop_back() noexcept(std::is_nothrow_destructible_v<Type_>) {

			using pointer_type = ptr<Type_>;
			cref<pointer_type> myLst { static_cast<pointer_type>(_last) };

			using allocator_type = std::allocator<Type_>;
			using allocator_traits = std::allocator_traits<allocator_type>;
			allocator_type alloc {};

			allocator_traits::destroy(alloc, myLst);
			_last = myLst - 1;
		}

		/**
		 * Will remove the first stored element
		 *
		 * @author Julius
		 * @date 27.08.2021
		 */
		void pop_front() noexcept {

			using pointer_type = decltype(_base);
			cref<pointer_type> myFst { static_cast<pointer_type>(_base) };

			if constexpr (Stable_) {
				cref<pointer_type> myLst { static_cast<pointer_type>(_last) };

				ptr<char> prev { static_cast<ptr<char>>(myFst) };
				ptr<char> next { prev + _proxy.align };
				for (; prev != myLst; prev = next, next += _proxy.align) {
					_proxy.move_assignment(static_cast<ptr<void>>(prev), static_cast<ptr<void>>(next));
				}

				_proxy.destructor(myLst);
				_last = static_cast<ptr<char>>(myLst) - _proxy.align;

			} else {
				cref<pointer_type> myLst { static_cast<pointer_type>(_last) };

				_proxy.move_assignment(myFst, myLst);
				_proxy.destructor(myLst);
				_last = static_cast<ptr<char>>(myLst) - _proxy.align;
			}
		}

		/**
		 * Will remove the first stored element
		 *
		 * @tparam Type_ Type of the type.
		 */
		template <typename Type_>
		void pop_front() noexcept(std::is_nothrow_move_assignable_v<Type_> && std::is_nothrow_destructible_v<Type_>) {

			using pointer_type = ptr<Type_>;
			cref<pointer_type> myFst { static_cast<pointer_type>(_base) };

			if constexpr (Stable_) {
				cref<pointer_type> myLst { static_cast<pointer_type>(_last) };

				pointer_type prev { myFst };
				pointer_type next { prev + 1 };
				for (; prev != myLst; prev = next, ++next) {
					*prev = std::move(*next);
				}

				using allocator_type = std::allocator<Type_>;
				using allocator_traits = std::allocator_traits<allocator_type>;
				allocator_type alloc {};

				allocator_traits::destroy(alloc, myLst);
				_last = myLst - 1;

			} else {
				cref<pointer_type> myLst { static_cast<pointer_type>(_last) };

				*myFst = std::move(*myLst);

				using allocator_type = std::allocator<Type_>;
				using allocator_traits = std::allocator_traits<allocator_type>;
				allocator_type alloc {};

				allocator_traits::destroy(alloc, myLst);
				_last = myLst - 1;
			}
		}

	public:
		void erase(ptr<void> where_) noexcept {

			using pointer_type = decltype(_base);
			cref<pointer_type> myLst { static_cast<pointer_type>(_last) };
			cref<pointer_type> newLst { static_cast<ptr<char>>(myLst) - _proxy.align };
			cref<pointer_type> wherePtr { static_cast<pointer_type>(where_) };

			#ifdef _DEBUG
			assert(_base <= wherePtr && wherePtr <= _last);
			#endif

			if constexpr (Stable_) {
				ptr<char> prev { static_cast<ptr<char>>(wherePtr) };
				ptr<char> next { prev + _proxy.align };

				for (; prev != myLst; prev = next, next += _proxy.align) {
					_proxy.move_assignment(prev, next);
				}

				if (_proxy.destructor) {
					_proxy.destructor(newLst);
				}
				_last = newLst;

			} else {
				_proxy.move_assignment(wherePtr, newLst);
				if (_proxy.destructor) {
					_proxy.destructor(newLst);
				}
				_last = newLst;
			}
		}

		void erase(ptr<void> first_, ptr<void> last_) noexcept {

			using pointer_type = decltype(_base);
			cref<pointer_type> myLst { static_cast<pointer_type>(_last) };
			cref<pointer_type> whereFst { static_cast<pointer_type>(first_) };
			cref<pointer_type> whereLst { static_cast<pointer_type>(last_) };

			ptr<char> writer { static_cast<ptr<char>>(whereFst) };
			ptr<char> reader { static_cast<ptr<char>>(whereLst) };

			for (; reader != myLst; writer += _proxy.align, reader += _proxy.align) {
				_proxy.move_assignment(static_cast<ptr<void>>(writer), static_cast<ptr<void>>(reader));
			}

			if (_proxy.destructor) {
				count_type remove {
					(static_cast<ptr<char>>(whereLst) - static_cast<ptr<char>>(whereFst)) / _proxy.align
				};
				ptr<char> lst { static_cast<ptr<char>>(myLst) - _proxy.align };

				for (; remove > 0; --remove, lst -= _proxy.align) {
					_proxy.destructor(static_cast<ptr<void>>(lst));
				}

				_last = lst + _proxy.align;

			} else {
				const size_type range = (static_cast<ptr<char>>(whereLst) - static_cast<ptr<char>>(whereFst));
				const size_type offrange = (static_cast<ptr<char>>(myLst) - _proxy.align - static_cast<ptr<char>>(
					whereLst));
				_last = static_cast<ptr<char>>(myLst) - (range - offrange);
			}
		}

		template <typename Type_>
		iterator_type<Type_> erase(const_iterator_type<Type_> where_) {

			using pointer_type = ptr<Type_>;
			cref<pointer_type> myLst { static_cast<pointer_type>(_last) };
			cref<pointer_type> newLst { myLst - 1 };
			cref<pointer_type> wherePtr { static_cast<pointer_type>(where_._cur) };

			#ifdef _DEBUG
			assert(_base <= wherePtr && wherePtr <= _last);
			#endif

			if constexpr (Stable_) {
				pointer_type prev { wherePtr };
				pointer_type next { prev + 1 };

				for (; prev != myLst; prev = next, ++next) {
					*prev = std::move(*next);
				}

				using allocator_type = std::allocator<Type_>;
				using allocator_traits = std::allocator_traits<allocator_type>;
				allocator_type alloc {};

				allocator_traits::destroy(alloc, newLst);
				_last = newLst;

			} else {
				*wherePtr = std::move(*newLst);

				using allocator_type = std::allocator<Type_>;
				using allocator_traits = std::allocator_traits<allocator_type>;
				allocator_type alloc {};

				allocator_traits::destroy(alloc, newLst);
				_last = newLst;
			}

			return { wherePtr };
		}

		template <typename Type_>
		iterator_type<Type_> erase(const_iterator_type<Type_> first_, const_iterator_type<Type_> last_) noexcept {

			using pointer_type = ptr<Type_>;
			cref<pointer_type> myLst { static_cast<pointer_type>(_last) };
			cref<pointer_type> whereFst { static_cast<pointer_type>(first_._cur) };
			cref<pointer_type> whereLst { static_cast<pointer_type>(last_._cur) };

			pointer_type writer { whereFst };
			pointer_type reader { whereLst };

			for (; reader != myLst; ++writer, ++reader) {
				*writer = std::move(*reader);
			}

			using allocator_type = std::allocator<Type_>;
			using allocator_traits = std::allocator_traits<allocator_type>;
			allocator_type alloc {};

			count_type remove { static_cast<size_type>(whereLst - whereFst) };
			pointer_type lst { myLst - 1 };

			for (; remove > 0; --remove, --lst) {
				allocator_traits::destroy(alloc, lst);
			}

			_last = ++lst;
			return { whereFst };
		}

	private:
		void copy_assign(const_reference_type other_) {

			using pointer_type = decltype(_base);
			cref<pointer_type> myFst { _base };
			cref<pointer_type> myLst { _last };

			ptr<char> fst { static_cast<ptr<char>>(other_._base) };
			cref<pointer_type> lst { static_cast<pointer_type>(other_._last) };

			/**
			 * Copy Assign in defined range
			 */
			ptr<char> next { static_cast<ptr<char>>(myFst) };
			for (; fst != lst && next != myLst; fst += _proxy.align, next += _proxy.align) {
				_proxy.copy_assignment(static_cast<ptr<void>>(next), static_cast<ptr<void>>(fst));
			}

			/**
			 * Append
			 */
			reserve(other_.size());

			for (; fst != lst; fst += _proxy.align) {
				push_back(static_cast<ptr<void>>(fst));
			}
		}

		template <typename Type_>
		void copy_assign(
			const_reference_type other_
		) noexcept(std::is_nothrow_copy_constructible_v<Type_> && std::is_nothrow_copy_assignable_v<Type_>) {

			using pointer_type = ptr<Type_>;
			cref<pointer_type> myFst { static_cast<pointer_type>(_base) };
			cref<pointer_type> myLst { static_cast<pointer_type>(_last) };

			pointer_type fst { static_cast<pointer_type>(other_._base) };
			cref<pointer_type> lst { static_cast<pointer_type>(other_._last) };

			/**
			 * Copy Assign in defined range
			 */
			pointer_type next { myFst };
			for (; fst != lst && next != myLst; ++fst, ++next) {
				*next = *fst;
			}

			/**
			 * Append
			 */
			reserve<Type_>(other_.size<Type_>());

			for (; fst != lst; ++fst) {
				push_back<Type_>(*fst);
			}
		}

	private:
		void swap_internal(reference_type other_) noexcept {
			_proxy = std::exchange(other_._proxy, _proxy);
			_base = std::exchange(other_._base, _base);
			_end = std::exchange(other_._end, _end);
			_last = std::exchange(other_._last, _last);
		}

		void move_assign_override(reference_type other_) {

			using pointer_type = decltype(_base);
			cref<pointer_type> myFst { _base };
			cref<pointer_type> myLst { _last };

			ptr<char> fst { static_cast<ptr<char>>(other_._base) };
			cref<pointer_type> lst { static_cast<pointer_type>(other_._last) };

			/**
			 * Move Assign in defined range
			 */
			ptr<char> next { static_cast<ptr<char>>(myFst) };
			for (; fst != lst && next != myLst; fst += _proxy.align, next += _proxy.align) {
				_proxy.move_assignment(static_cast<ptr<void>>(next), static_cast<ptr<void>>(fst));
			}

			/**
			 * Append
			 */
			reserve(other_.size());

			for (; fst != lst; fst += _proxy.align) {
				push_back(std::move(static_cast<ptr<void>>(fst)));
			}

			/**
			 * Cleanup
			 */
			other_.tidy();
		}

		template <typename Type_>
		void move_assign_override(
			reference_type other_
		) noexcept(std::is_nothrow_move_constructible_v<Type_> && std::is_nothrow_move_assignable_v<Type_>) {

			using pointer_type = ptr<Type_>;
			cref<pointer_type> myFst { static_cast<pointer_type>(_base) };
			cref<pointer_type> myLst { static_cast<pointer_type>(_last) };

			pointer_type fst { static_cast<pointer_type>(other_._base) };
			cref<pointer_type> lst { static_cast<pointer_type>(other_._last) };

			/**
			 * Move Assign in defined range
			 */
			pointer_type next { myFst };
			for (; fst != lst && next != myLst; ++fst, ++next) {
				*next = std::move(*fst);
			}

			/**
			 * Append
			 */
			reserve<Type_>(other_.size<Type_>());

			for (; fst != lst; ++fst) {
				push_back<Type_>(std::move(*fst));
			}

			/**
			 * Cleanup
			 */
			other_.tidy<Type_>();
		}

	private:
		void realloc_base_r(
			const count_type oldSize_,
			const count_type newSize_,
			const count_type newCapacity_,
			cref<ptr<void>> value_
		) {

			using pointer_type = decltype(_base);

			pointer_type newFst, reallocBase;
			newFst = reallocBase = std::realloc(_base, newCapacity_ * _proxy.align);

			if (reallocBase == nullptr) {
				newFst = std::malloc(newCapacity_ * _proxy.align);

				ptr<char> myFst { static_cast<ptr<char>>(_base) };
				cref<pointer_type> myLst { static_cast<pointer_type>(_last) };

				ptr<char> next { static_cast<ptr<char>>(newFst) };
				for (; myFst != myLst; myFst += _proxy.align, next += _proxy.align) {
					_proxy.move_constructor(static_cast<ptr<void>>(next), static_cast<ptr<void>>(myFst));
				}
			}

			ptr<char> next { static_cast<ptr<char>>(newFst) };
			const pointer_type newLst = next + newSize_ * _proxy.align;

			next += oldSize_ * _proxy.align;
			for (; next != newLst; next += _proxy.align) {
				_proxy.copy_constructor(static_cast<ptr<void>>(next), value_);
			}

			if (reallocBase != newFst && _base) {
				destroy_range(_base, _last);
				std::free(_base);
			}

			constexpr std::false_type _no_auto_dealloc_ {};
			change_internal(newFst, newSize_, newCapacity_, _no_auto_dealloc_);
		}

		void alloc_base_r(
			const count_type oldSize_,
			const count_type newSize_,
			const count_type newCapacity_,
			cref<ptr<void>> value_
		) {

			using pointer_type = decltype(_base);

			const pointer_type newFst = std::malloc(newCapacity_ * _proxy.align);
			const pointer_type prevLst = static_cast<ptr<char>>(newFst) + oldSize_ * _proxy.align;
			const pointer_type newLst = static_cast<ptr<char>>(newFst) + newSize_ * _proxy.align;

			ptr<char> myFst { static_cast<ptr<char>>(_base) };
			cref<pointer_type> myLst { static_cast<pointer_type>(_last) };

			ptr<char> next { static_cast<ptr<char>>(newFst) };
			for (; myFst != myLst; myFst += _proxy.align, next += _proxy.align) {
				_proxy.move_constructor(static_cast<ptr<void>>(next), static_cast<ptr<void>>(myFst));
			}

			next = static_cast<ptr<char>>(prevLst);
			for (; next != newLst; next += _proxy.align) {
				_proxy.copy_constructor(static_cast<ptr<void>>(next), value_);
			}

			change_internal(newFst, newSize_, newCapacity_);
		}

		void reallocate_resize(const count_type size_, cref<ptr<void>> value_) {

			const count_type oldSize = size();
			const count_type newCapacity = calculate_grow(size_);

			if constexpr (Realloc_) {
				realloc_base_r(oldSize, size_, newCapacity, value_);

			} else {
				alloc_base_r(oldSize, size_, newCapacity, value_);
			}
		}

		template <typename Type_> requires std::is_move_constructible_v<Type_> && std::is_copy_constructible_v<Type_>
		void reallocate_resize(const size_type size_, cref<Type_> value_) {

			#ifdef _DEBUG
			assert(sizeof(Type_) == _proxy.align);
			#endif

			using pointer_type = ptr<Type_>;

			cref<pointer_type> myFst { static_cast<pointer_type>(_base) };
			cref<pointer_type> myLst { static_cast<pointer_type>(_last) };

			const count_type oldSize = size<Type_>();
			const count_type newCapacity = calculate_grow(size_);

			using allocator_type = std::allocator<Type_>;
			using allocator_traits = std::allocator_traits<allocator_type>;

			allocator_type alloc {};
			const pointer_type newFst = alloc.allocate(newCapacity);
			const pointer_type prevLst = newFst + oldSize;

			std::uninitialized_move(myFst, myLst, newFst/* TODO: , alloc */);
			std::uninitialized_fill_n(prevLst, oldSize - size_, value_/* TODO: , alloc */);

			change_internal<Type_>(newFst, size_, newCapacity);
		}

		template <typename Type_> requires std::is_copy_constructible_v<Type_>
		void resize_internal(const count_type size_, cref<Type_> value_) {

			using pointer_type = ptr<Type_>;

			cref<pointer_type> myFst { static_cast<pointer_type>(_base) };
			ref<pointer_type> myLst { static_cast<pointer_type>(_last) };

			const count_type oldSize = size<Type_>();

			/**
			 * Check for trimming
			 */
			if (size_ < oldSize) {
				const pointer_type newLst = myFst + size_;
				destroy_range<Type_>(newLst, myLst);
				myLst = newLst;
				return;
			}

			/**
			 * Check for Append
			 */
			if (size_ > oldSize) {
				const size_type oldCapacity = capacity<Type_>();
				if (size_ > oldCapacity) {
					reallocate_resize<Type_>(size_, value_);
					return;
				}

				using allocator_type = std::allocator<Type_>;
				allocator_type alloc {};

				const pointer_type oldLst = myLst;
				myLst = std::uninitialized_fill_n(oldLst, size_ - oldSize, value_/* TODO: , alloc */);
			}
		}

		void realloc_base(const count_type size_, const count_type newCapacity_) {

			using pointer_type = decltype(_base);

			pointer_type newFst, reallocBase;
			newFst = reallocBase = std::realloc(_base, newCapacity_ * _proxy.align);

			if (reallocBase == nullptr) {
				newFst = std::malloc(newCapacity_ * _proxy.align);

				ptr<char> myFst { static_cast<ptr<char>>(_base) };
				cref<pointer_type> myLst { static_cast<pointer_type>(_last) };

				ptr<char> next { static_cast<ptr<char>>(newFst) };
				for (; myFst != myLst; myFst += _proxy.align, next += _proxy.align) {
					_proxy.move_constructor(static_cast<ptr<void>>(next), static_cast<ptr<void>>(myFst));
				}
			}

			if (reallocBase != newFst && _base) {
				destroy_range(_base, _last);
				std::free(_base);
			}

			constexpr std::false_type _no_auto_dealloc {};
			change_internal(newFst, size_, newCapacity_, _no_auto_dealloc);
		}

		void alloc_base(const count_type size_, const count_type newCapacity_) {

			using pointer_type = decltype(_base);

			const pointer_type newFst = std::malloc(newCapacity_ * _proxy.align);

			ptr<char> myFst { static_cast<ptr<char>>(_base) };
			cref<pointer_type> myLst { static_cast<pointer_type>(_last) };

			ptr<char> next { static_cast<ptr<char>>(newFst) };
			for (; myFst != myLst; myFst += _proxy.align, next += _proxy.align) {
				_proxy.move_constructor(static_cast<ptr<void>>(next), static_cast<ptr<void>>(myFst));
			}

			change_internal(newFst, size_, newCapacity_);
		}

		void reallocate_exactly(const count_type capacity_) {

			const count_type oldSize = size();

			if constexpr (Realloc_) {
				realloc_base(oldSize, capacity_);

			} else {
				alloc_base(oldSize, capacity_);
			}
		}

	public:
		/**
		 * Reserves at least the given capacity_ without mutating the size
		 *
		 * @author Julius
		 * @date 27.08.2021
		 *
		 * @param  newCapacity_ The new capacity.
		 */
		void reserve(count_type newCapacity_) {
			if (newCapacity_ > capacity()) {
				reallocate_exactly(newCapacity_);
			}
		}

		/**
		 * Reserves at least the given capacity_ without mutating the size
		 *
		 * @tparam Type_ Type of the type.
		 * @param  newCapacity_ The new capacity.
		 */
		template <typename Type_> requires std::is_move_constructible_v<Type_>
		void reserve(count_type newCapacity_) {
			if (newCapacity_ > capacity<Type_>()) {
				reallocate_exactly<Type_>(newCapacity_);
			}
		}

		void resize(count_type newSize_, ptr<void> value_) {
			resize_internal(newSize_, value_);
		}

		template <typename Type_> requires std::is_default_constructible_v<Type_> && std::is_copy_constructible_v<Type_>
		void resize(count_type newSize_) {
			resize_internal<Type_>(newSize_, {});
		}

		template <typename Type_> requires std::is_copy_constructible_v<Type_>
		void resize(count_type newSize_, cref<Type_> value_) {
			resize_internal<Type_>(newSize_, value_);
		}

		/**
		 * Reduces capacity to required size to hold elements
		 *
		 * @author Julius
		 * @date 27.08.2021
		 */
		void shrink_to_fit() {
			using pointer_type = decltype(_base);

			cref<pointer_type> myLst { static_cast<pointer_type>(_last) };
			cref<pointer_type> myEnd { static_cast<pointer_type>(_end) };
			if (myLst != myEnd) {
				cref<pointer_type> myFst { static_cast<pointer_type>(_base) };
				if (myFst == myLst) {
					tidy();
				} else {
					reallocate_exactly(size());
				}
			}
		}

		/**
		 * Reduces capacity to required size to hold elements
		 *
		 * @tparam Type_ Type of the type.
		 *
		 * ### author Julius
		 * ### date 27.08.2021
		 */
		template <typename Type_> requires std::is_move_constructible_v<Type_>
		void shrink_to_fit() {
			using pointer_type = ptr<Type_>;

			cref<pointer_type> myLst { static_cast<pointer_type>(_last) };
			cref<pointer_type> myEnd { static_cast<pointer_type>(_end) };
			if (myLst != myEnd) {
				cref<pointer_type> myFst { static_cast<pointer_type>(_base) };
				if (myFst == myLst) {
					tidy<Type_>();
				} else {
					reallocate_exactly<Type_>(size<Type_>());
				}
			}
		}

	public:
		reference_type operator=(const_reference_type other_) {
			if (this != std::addressof(other_)) {
				copy_assign(other_);
			}
			return *this;
		}

		template <typename Type_>
		reference_type operator=(const_reference_type other_) {
			if (this != std::addressof(other_)) {
				copy_assign<Type_>(other_);
			}
			return *this;
		}

		reference_type operator=(mref<value_type> other_) noexcept {
			if (this != std::addressof(other_)) {
				// move_assign_override(other_);
				swap_internal(other_);
			}
			return *this;
		}

		template <typename Type_>
		reference_type operator=(mref<value_type> other_) noexcept {
			if (this != std::addressof(other_)) {
				// move_assign_override<Type_>(other_);
				swap_internal(other_);
			}
			return *this;
		}

		void swap(const_reference_type other_) noexcept {
			if (this != std::addressof(other_)) {
				swap_internal(other_);
			}
		}

		static void swap(const_reference_type left_, const_reference_type right_) noexcept {
			left_.swap_internal(right_);
		}

	public:
		/**
		 * Subscribe operator
		 *
		 * @author Julius
		 * @date 27.08.2021
		 *
		 * @param  pos_ The position.
		 *
		 * @returns A const reference to the value.
		 */
		template <typename Type_>
		[[nodiscard]] cref<Type_> operator[](const count_type pos_) const noexcept {
			return static_cast<ptr<Type_>>(_base)[pos_];
		}

		/**
		 * Subscribe operator
		 *
		 * @author Julius
		 * @date 27.08.2021
		 *
		 * @param  pos_ The position.
		 *
		 * @returns A reference to the value.
		 */
		template <typename Type_>
		[[nodiscard]] ref<Type_> operator[](const count_type pos_) noexcept {
			return static_cast<ptr<Type_>>(_base)[pos_];
		}

		/**
		 * Lookup value the given pos_
		 *
		 * @exception std::Thrown when pos_ is out of range.
		 *
		 * @tparam Type_ Type of the type.
		 * @param  pos_ The position.
		 *
		 * @returns A const reference to the value.
		 */
		template <typename Type_>
		[[nodiscard]] cref<Type_> at(const count_type pos_) const {
			if (pos_ > size()) {
				throw std::out_of_range("invalid vector subscript");
			}
			return static_cast<ptr<Type_>>(_base)[pos_];
		}

		/**
		 * Lookup value the given pos_
		 *
		 * @exception std::Thrown when pos_ is out of range.
		 *
		 * @tparam Type_ Type of the type.
		 * @param  pos_ The position.
		 *
		 * @returns A reference to the value.
		 */
		template <typename Type_>
		[[nodiscard]] ref<Type_> at(const count_type pos_) {
			if (pos_ > size()) {
				throw std::out_of_range("invalid vector subscript");
			}
			return static_cast<ptr<Type_>>(_base)[pos_];
		}
	};

	template <
		typename Type_,
		typename SizeType_,
		typename GrowthPolicy_,
		bool Stable_,
		bool Realloc_
	>
	class TypedProxyVector :
		public ProxyVector<SizeType_, GrowthPolicy_, Stable_, Realloc_> {
	private:
		using base_type = ProxyVector<SizeType_, GrowthPolicy_, Stable_, Realloc_>;

		using value_type = TypedProxyVector<Type_, SizeType_, GrowthPolicy_, Stable_, Realloc_>;
		using reference_type = ref<value_type>;
		using const_reference_type = cref<value_type>;

		using iterator_type = typename base_type::template iterator_type<Type_>;
		using const_iterator_type = typename base_type::template const_iterator_type<Type_>;

		/**
		 * Prevent unwanted copy or move assignment or constructor invokes
		 */
	public:
		TypedProxyVector() = delete;

		TypedProxyVector(cref<TypedProxyVector>) = delete;

		TypedProxyVector(mref<TypedProxyVector>) noexcept = delete;

		~TypedProxyVector() = delete;

		reference_type operator=(const_reference_type) = delete;

		reference_type operator=(mref<value_type>) noexcept = delete;

	public:
		[[nodiscard]] typename base_type::count_type capacity() const noexcept {
			return this->base_type::template capacity<Type_>();
		}

		[[nodiscard]] typename base_type::count_type size() const noexcept {
			return this->base_type::template size<Type_>();
		}

	public:
		[[nodiscard]] iterator_type begin() noexcept {
			return this->base_type::template begin<Type_>();
		}

		[[nodiscard]] const_iterator_type begin() const noexcept {
			return this->base_type::template begin<Type_>();
		}

		[[nodiscard]] const_iterator_type cbegin() const noexcept {
			return this->base_type::template cbegin<Type_>();
		}

		[[nodiscard]] iterator_type end() noexcept {
			return this->base_type::template end<Type_>();
		}

		[[nodiscard]] const_iterator_type end() const noexcept {
			return this->base_type::template end<Type_>();
		}

		[[nodiscard]] const_iterator_type cend() const noexcept {
			return this->base_type::template cend<Type_>();
		}

	public:
		void push_back(mref<Type_> value_) noexcept(std::is_nothrow_move_constructible_v<Type_>) {
			return this->base_type::template push_back<Type_>(std::forward<Type_>(value_));
		}

		void push_back(cref<Type_> value_) noexcept(std::is_nothrow_copy_constructible_v<Type_>) {
			return this->base_type::template push_back<Type_>(value_);
		}

		[[nodiscard]] iterator_type emplace_back(
			mref<Type_> value_
		) noexcept (std::is_nothrow_move_constructible_v<Type_>) {
			return this->base_type::template emplace_back<Type_>(std::forward<Type_>(value_));
		}

		[[nodiscard]] iterator_type emplace_back(
			cref<Type_> value_
		) noexcept (std::is_nothrow_copy_constructible_v<Type_>) {
			return this->base_type::template emplace_back<Type_>(value_);
		}

	public:
		void pop_back() noexcept(std::is_nothrow_destructible_v<Type_>) {
			this->base_type::template pop_back<Type_>();
		}

		void pop_front() noexcept(std::is_nothrow_move_assignable_v<Type_> &&
			std::is_nothrow_destructible_v<Type_>) {
			this->base_type::template pop_front<Type_>();
		}

	public:
		iterator_type erase(const_iterator_type where_) noexcept {
			return this->base_type::template erase<Type_>(where_);
		}

		iterator_type erase(const_iterator_type first_, const_iterator_type last_) noexcept {
			return this->base_type::template erase<Type_>(first_, last_);
		}

	public:
		void reserve(const typename base_type::count_type newCapacity_) {
			this->base_type::template reserve<Type_>(newCapacity_);
		}

		void resize(const typename base_type::count_type newSize_) {
			this->base_type::template resize<Type_>(newSize_);
		}

		void resize(const typename base_type::count_type newSize_, cref<Type_> value_) {
			this->base_type::template resize<Type_>(newSize_, value_);
		}

		void shrink_to_fit() noexcept(std::is_nothrow_move_assignable_v<Type_>) {
			this->base_type::template shrink_to_fit<Type_>();
		}

	public:
		[[nodiscard]] cref<Type_> operator[](const typename base_type::count_type pos_) const noexcept {
			return this->base_type::template operator[]<Type_>(pos_);
		}

		[[nodiscard]] ref<Type_> operator[](const typename base_type::count_type pos_) noexcept {
			return this->base_type::template operator[]<Type_>(pos_);
		}

		[[nodiscard]] cref<Type_> at(const typename base_type::count_type pos_) const {
			return this->base_type::template at<Type_>(pos_);
		}

		[[nodiscard]] ref<Type_> at(const typename base_type::count_type pos_) {
			return this->base_type::template at<Type_>(pos_);
		}
	};
}
