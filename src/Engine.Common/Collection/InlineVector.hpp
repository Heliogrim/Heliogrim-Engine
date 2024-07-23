#pragma once

#include <cassert>
#include <memory>

#include "InlineArray.hpp"
#include "../Wrapper.hpp"

namespace hg {
	template <class Type_, size_t Capacity_ = 1, typename Allocator_ = std::allocator<Type_>>
	class InlineVector {
	public:
		using this_type = InlineVector<Type_, Capacity_, Allocator_>;

		static constexpr auto element_align = alignof(Type_);
		static constexpr auto element_size = sizeof(Type_);

		static constexpr auto inline_byte_size = element_size * Capacity_;

		using size_type = size_t;
		using difference_type = ptrdiff_t;

		using allocator_type = Allocator_;
		using allocator_traits = std::allocator_traits<allocator_type>;

	public:
		constexpr InlineVector() noexcept;

		constexpr ~InlineVector() noexcept {
			tidy();
		}

	private:
		allocator_type _alloc;
		allocator_traits _traits;

		_::byte _inlineStorage[inline_byte_size];
		ptr<Type_> _inlineEnd;

		_::byte _externalStorage;
		ptr<Type_> _externalLast;
		ptr<Type_> _externalEnd;

	protected:
		constexpr void tidy() {

			/* Destroy inline allocated objects */

			destroy_range(inline_begin(), _inlineEnd, _alloc);

			/* Destroy external allocated objects */

			destroy_range(external_begin(), _externalLast, _alloc);
			_traits.deallocate(_alloc, _externalStorage, std::distance(external_begin(), _externalEnd));

			_externalStorage = _externalEnd = nullptr;
		}

		[[nodiscard]] constexpr ptr<Type_> inline_begin() const noexcept {
			return _inlineStorage;
		}

		[[nodiscard]] constexpr ptr<Type_> inline_last() const noexcept {
			return _inlineEnd != _inlineStorage ? _inlineEnd - 1 : _inlineEnd;
		}

		[[nodiscard]] constexpr size_type inline_size() const noexcept {
			return std::distance(inline_begin(), _inlineEnd);
		}

		[[nodiscard]] constexpr ptr<Type_> external_begin() const noexcept {
			return _externalStorage;
		}

		[[nodiscard]] constexpr size_type external_capacity() const noexcept {
			return std::distance(external_begin(), _externalEnd);
		}

		[[nodiscard]] constexpr size_t external_size() const noexcept {
			return std::distance(external_begin(), _externalLast);
		}

	protected:
		constexpr bool inline_emplace_back(auto&&... args_) {

			const auto diff = inline_size();
			if (diff >= Capacity_) {
				return false;
			}

			_traits.construct(_alloc, _inlineEnd, std::forward<decltype(args_)>(args_)...);
			return ++_inlineEnd;
		}

		constexpr void inline_pop_back() {
			assert(_inlineEnd != _inlineStorage);
			_traits.destroy(_alloc, --_inlineEnd);
		}

		constexpr void external_emplace_back(auto&&... args_) {
			_traits.construct(_alloc, _externalLast, std::forward<decltype(args_)>(args_)...);
			++_externalLast;
		}

		constexpr void _Xchange_external(const ptr<Type_> newPt_, const size_type newSize_, const size_type newCap_) {

			if (external_begin()) {
				destroy_range(external_begin(), _externalLast, _alloc);
				_alloc.deallocate(_externalStorage);
			}

			_externalStorage = newPt_;
			_externalLast = newPt_ + newSize_;
			_externalEnd = newPt_ + newCap_;
		}

		constexpr void external_realloc_emplace(auto&&... args_) {

			const auto oldSize = external_size();

			auto [newPt_, newCap_] = _traits.allocate_at_least(_alloc, external_capacity() + 1);

			if (_externalLast == _externalEnd) {
				if constexpr (std::is_nothrow_move_constructible_v<Type_> || not std::is_copy_constructible_v<Type_>) {
					std::uninitialized_move(external_begin(), _externalLast, newPt_/* TODO: , _alloc*/);
				} else {
					std::uninitialized_copy(external_begin(), _externalLast, newPt_/* TODO: , _alloc*/);
				}
			}

			_traits.construct(_alloc, newPt_ + oldSize, std::forward<decltype(args_)>(args_)...);
			_Xchange_external(newPt_, oldSize + 1, newCap_);
		}

		[[nodiscard]] constexpr bool external_pop_back() {

			if (_externalLast != _externalStorage) {
				_traits.destroy(_alloc, --_externalLast);
				return true;
			}

			return false;
		}

	public:
		[[nodiscard]] size_type size() const noexcept {
			return inline_size() + external_size();
		}

		[[nodiscard]] size_type capacity() const noexcept {
			return (Capacity_ - inline_size()) + (external_capacity() - external_size());
		}

		[[nodiscard]] constexpr auto begin() const noexcept {
			// Warning:
			// TODO:
			return inline_begin();
		}

		[[nodiscard]] constexpr auto begin() noexcept {
			// Warning:
			// TODO:
			return inline_begin();
		}

		[[nodiscard]] constexpr auto cbegin() noexcept {
			// Warning:
			// TODO:
			return inline_begin();
		}

		[[nodiscard]] constexpr auto end() const noexcept {
			// Warning:
			// TODO:
			return _inlineEnd;
		}

		[[nodiscard]] constexpr auto end() noexcept {
			// Warning:
			// TODO:
			return _inlineEnd;
		}

		[[nodiscard]] constexpr auto cend() noexcept {
			// Warning:
			// TODO:
			return _inlineEnd;
		}

	public:
		void emplace_back(auto&&... val_) {

			if (inline_emplace_back(val_)) {
				return;
			}

			if (external_capacity() > external_size()) {
				external_emplace_back(std::forward<decltype(val_)>(val_)...);
				return;
			}

			external_realloc_emplace(std::forward<decltype(val_)>(val_)...);
		}

		void pop_back() {

			if (external_pop_back()) {
				return;
			}

			inline_pop_back();
		}

		template <typename Iter_>
		constexpr Iter_ erase(const Iter_& where_) noexcept;

		constexpr void clear() {
			tidy();
		}

		constexpr void reserve(size_type reserved_) {
			if (reserved_ > size()) {
				// TODO:
			}
		}

		constexpr void resize(size_t newSize_);

		constexpr void resize(size_t newSize_, auto&&... args_);
	};
}
