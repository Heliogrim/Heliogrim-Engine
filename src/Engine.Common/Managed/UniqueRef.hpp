#pragma once

#include <functional>

#include "../Move.hpp"
#include "../Optional.hpp"
#include "../Wrapper.hpp"

namespace hg {
	template <typename Type_>
	class UniqueRef final {
	public:
		using data_type = ::hg::meta::drop_ref_t<Type_>;
		using this_type = UniqueRef<data_type>;

		constexpr static auto is_data_const = std::is_const_v<Type_>;

	public:
		constexpr UniqueRef() noexcept :
			_data(),
			_release() {}

		constexpr explicit UniqueRef(auto&& release_, ref<data_type> data_) noexcept requires
			std::is_invocable_v<std::decay_t<decltype(release_)>, ref<data_type>> :
			_data(Some<ref<data_type>>(data_)),
			_release(::hg::forward<decltype(release_)>(release_)) {}

		constexpr UniqueRef(mref<this_type> other_) noexcept :
			_data(other_._data.take()),
			_release(::hg::move(other_._release)) {}

		UniqueRef(ref<const this_type>) = delete;

		constexpr ~UniqueRef() {
			release();
		}

	public:
		constexpr ref<this_type> operator=(mref<this_type> other_) noexcept {
			if (std::addressof(other_) == this) {
				return *this;
			}

			release();
			_data = ::hg::move(other_._data);
			_release = ::hg::move(other_._release);
			other_._data.reset();

			return *this;
		}

		ref<this_type> operator=(ref<const this_type>) = delete;

	private:
		Opt<ref<data_type>> _data;
		std::function<void(ref<data_type> obj_)> _release;

	public:
		[[nodiscard]] constexpr auto is_null() const noexcept {
			return _data.is_null();
		}

		[[nodiscard]] constexpr decltype(auto) value() const noexcept {
			return _data.value();
		}

		[[nodiscard]] ref<data_type> operator*() const noexcept requires is_data_const {
			return *_data;
		}

		[[nodiscard]] ref<data_type> operator*() const noexcept requires (not is_data_const) {
			return const_cast<ref<data_type>>(*_data);
		}

		[[nodiscard]] const ptr<data_type> operator->() const noexcept requires is_data_const {
			return &*_data;
		}

		[[nodiscard]] const ptr<data_type> operator->() const noexcept requires (not is_data_const) {
			return &const_cast<ref<data_type>>(*_data);
		}

		constexpr void release() {
			if (not _data.is_null()) {
				_release(*_data);
				_data.reset();
			}
		}
	};

	template <typename Type_>
	UniqueRef(auto&& fn_, ref<Type_>) -> UniqueRef<Type_>;
}
