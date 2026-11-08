#pragma once

#include <type_traits>
#include <utility>
#include <Engine.Asserts/Asserts.hpp>

#include "Future.hpp"

namespace hg {
	template <typename T_, typename E_ = Broken>
	class Promise {
	public:
		using state_type = detail::FutureState<T_, E_>;

	public:
		Promise() :
			_state(Arci<state_type>::create()) {}

		Promise(const Promise&) = delete;

		Promise(Promise&& other_) noexcept :
			_state(std::move(other_._state)),
			_futureTaken(other_._futureTaken),
			_finished(other_._finished) {}

		~Promise() {
			drop();
		}

	public:
		Promise& operator=(const Promise&) = delete;

		Promise& operator=(Promise&& other_) noexcept {
			if (this != std::addressof(other_)) {
				drop();
				_state = std::move(other_._state);
				_futureTaken = other_._futureTaken;
				_finished = other_._finished;
			}
			return *this;
		}

	private:
		Arci<state_type> _state;
		bool _futureTaken = false;
		bool _finished = false;

	public:
		[[nodiscard]] Future<T_, E_> future() {
			::hg::assertd(not _futureTaken);
			::hg::assertd(_state != nullptr);
			_futureTaken = true;
			return Future<T_, E_> { Arci<state_type> { _state } };
		}

		template <typename V_ = T_> requires (not std::is_void_v<V_>)
		void resolve(std::type_identity_t<V_>&& value_) noexcept {
			::hg::assertd(not _finished);
			_state->slot.template emplace<Resolved<T_>>(Resolved<T_> { std::move(value_) });
			finish();
		}

		void resolve() noexcept requires std::is_void_v<T_> {
			::hg::assertd(not _finished);
			_state->slot.template emplace<Resolved<void>>();
			finish();
		}

		void reject(E_&& error_) noexcept {
			::hg::assertd(not _finished);
			_state->slot.template emplace<Failed<E_>>(Failed<E_> { Fail<E_> { std::move(error_) } });
			finish();
		}

		[[nodiscard]] bool isPending() const noexcept {
			return _state != nullptr && not _finished;
		}

	private:
		void drop() noexcept {
			if (_state != nullptr && not _finished) {
				_state->slot.template emplace<Failed<E_>>(
					Failed<E_> { Fail<E_> { Broken { Broken::Reason::eDropped } } }
				);
				finish();
			}
			_state.reset();
		}

		void finish() noexcept {
			_finished = true;
			_state->done.set();
			// Keep the state while the future is untaken, so future() after completion still works.
			if (_futureTaken) {
				_state.reset();
			}
		}
	};
}
