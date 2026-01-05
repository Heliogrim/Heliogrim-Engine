#pragma once

#include <Engine.Common/Move.hpp>
#include <Engine.Common/Wrapper.hpp>

namespace hg::engine::reflow {
	template <typename Type_>
	class LayoutAttribute {
	public:
		using this_type = LayoutAttribute<Type_>;
		using store_type = meta::remove_reference_t<Type_>;

	public:
		LayoutAttribute() = default;

		constexpr explicit LayoutAttribute(auto&& first_, auto&&... rest_)
			requires std::is_constructible_v<store_type, mref<decltype(first_)>, mref<decltype(rest_)>...> :
			_store(::hg::forward<decltype(first_)>(first_), ::hg::forward<decltype(rest_)>(rest_)...) {}

		constexpr LayoutAttribute(cref<this_type> other_) noexcept :
			_store(other_._store) {}

		constexpr LayoutAttribute(mref<this_type> other_) noexcept :
			_store(::hg::move(other_._store)) {}

		constexpr ~LayoutAttribute() = default;

	public:
		ref<this_type> operator=(cref<this_type> other_) noexcept {
			if (std::addressof(other_) != this) {
				_store = other_._store;
			}
			return *this;
		}

		ref<this_type> operator=(mref<this_type> other_) noexcept {
			if (std::addressof(other_) != this) {
				_store = ::hg::move(other_._store);
			}
			return *this;
		}

	private:
		store_type _store;

	public:
		bool update(auto&& next_) requires std::is_same_v<std::remove_cvref_t<decltype(next_)>, Type_> {
			if constexpr (std::equality_comparable<Type_>) {
				if (next_ != _store) {
					_store = ::hg::forward<decltype(next_)&&>(next_);
					return true;
				}
				return false;

			} else {
				_store = ::hg::forward<decltype(next_)&&>(next_);
				return true;
			}
		}

		bool update(auto&& fn_) requires std::is_same_v<std::invoke_result_t<decltype(fn_), ref<const Type_>>, Type_> {
			auto next = std::forward<decltype(fn_)>(fn_)(_store);
			if constexpr (std::equality_comparable<Type_>) {
				if (next != _store) {
					_store = ::hg::move(next);
					return true;
				}
				return false;

			} else {
				_store = ::hg::move(next);
				return true;
			}
		}

		[[nodiscard]] ref<const Type_> value() const noexcept {
			return _store;
		}
	};
}
