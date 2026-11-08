#pragma once

#include <concepts>
#include <cstddef>
#include <memory>
#include <new>
#include <type_traits>
#include <utility>
#include <Engine.Asserts/Asserts.hpp>
#include <Engine.Common/Types.hpp>
#include <Engine.Common/Wrapper.hpp>

namespace hg {
	template <typename Sig_, size_t Storage_ = 64>
	class UniqueFn;

	/**
	 * Move-only, call-once callable with fixed inline storage and no heap fallback.
	 *
	 * Constructing from a callable that does not fit is rejected via constraint (so the trait is false):
	 * "capture too large for UniqueFn: move it into an Arci"
	 */
	template <typename R_, typename... Args_, size_t Storage_>
	class UniqueFn<R_(Args_...), Storage_> {
	private:
		enum class Op_ : unsigned char {
			eMove,
			eDestroy
		};

		template <typename Fn_>
		constexpr static bool fits_ = sizeof(std::decay_t<Fn_>) <= Storage_ &&
			alignof(std::decay_t<Fn_>) <= alignof(std::max_align_t);

	public:
		UniqueFn() noexcept = default;

		template <typename Fn_> requires (not std::same_as<std::remove_cvref_t<Fn_>, UniqueFn>) &&
			fits_<Fn_> &&
			std::is_invocable_r_v<R_, std::decay_t<Fn_>&, Args_...> &&
			std::constructible_from<std::decay_t<Fn_>, Fn_>
		UniqueFn(Fn_&& fn_) :
			_invoke(
				[](const ptr<void> self_, Args_&&... args_) -> R_ {
					return (*static_cast<ptr<std::decay_t<Fn_>>>(self_))(::hg::forward<Args_>(args_)...);
				}
			),
			_manage(
				[](const ptr<void> dst_, const ptr<void> src_, const Op_ op_) {
					using Stored = std::decay_t<Fn_>;
					if (op_ == Op_::eMove) {
						::new(dst_) Stored(std::move(*static_cast<ptr<Stored>>(src_)));
					}
					static_cast<ptr<Stored>>(src_)->~Stored();
				}
			) {
			::new(static_cast<ptr<void>>(_storage)) std::decay_t<Fn_>(::hg::forward<Fn_>(fn_));
		}

		UniqueFn(const UniqueFn&) = delete;

		UniqueFn(UniqueFn&& other_) noexcept {
			_take(other_);
		}

		~UniqueFn() {
			_reset();
		}

	public:
		UniqueFn& operator=(const UniqueFn&) = delete;

		UniqueFn& operator=(UniqueFn&& other_) noexcept {
			if (this != std::addressof(other_)) {
				_reset();
				_take(other_);
			}
			return *this;
		}

	public:
		[[nodiscard]] explicit operator bool() const noexcept {
			return _invoke != nullptr;
		}

		R_ operator()(Args_... args_) && {
			::hg::assertd(_invoke != nullptr);
			// Detach first so the callable is destroyed even if it throws; result is returned by value.
			UniqueFn local { std::move(*this) };
			return local._invoke(local._storage, ::hg::forward<Args_>(args_)...);
		}

	private:
		alignas(std::max_align_t) std::byte _storage[Storage_];

		R_ (*_invoke)(ptr<void>, Args_&&...) = nullptr;

		void (*_manage)(ptr<void>, ptr<void>, Op_) = nullptr;

	private:
		void _take(UniqueFn& other_) noexcept {
			if (other_._invoke == nullptr) {
				return;
			}

			other_._manage(_storage, other_._storage, Op_::eMove);
			_invoke = std::exchange(other_._invoke, nullptr);
			_manage = std::exchange(other_._manage, nullptr);
		}

		void _reset() noexcept {
			if (_invoke != nullptr) {
				_manage(nullptr, _storage, Op_::eDestroy);
				_invoke = nullptr;
				_manage = nullptr;
			}
		}
	};
}
