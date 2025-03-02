#pragma once

#include <Engine.Common/Move.hpp>
#include <Engine.Common/Wrapper.hpp>

namespace hg::engine::reflow {
	template <typename Type_>
	struct DataAttribute {
		using this_type = DataAttribute<Type_>;
		using store_type = meta::remove_reference_t<Type_>;

		store_type _store;

		bool update(auto&& next_) requires std::is_same_v<std::remove_cvref_t<decltype(next_)>, Type_> {
			if constexpr (std::equality_comparable<Type_>) {
				if (next_ != _store) {
					_store = ::hg::forward<decltype(next_)&&>(next_);
					return true;
				}
				return false;
			}
			_store = ::hg::forward<decltype(next_)&&>(next_);
			return true;
		}

		bool update(auto&& fn_) requires std::is_same_v<std::invoke_result_t<decltype(fn_), ref<const Type_>>, Type_> {
			auto next = std::forward<decltype(fn_)>(fn_)(_store);
			if constexpr (std::equality_comparable<Type_>) {
				if (next != _store) {
					_store = ::hg::move(next);
					return true;
				}
				return false;
			}
			_store = ::hg::move(next);
			return true;
		}

		[[nodiscard]] ref<const Type_> value() const noexcept {
			return _store;
		}
	};
}
