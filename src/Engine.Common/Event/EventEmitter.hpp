#pragma once
#include "../Inc.hpp"
#include "../String.hpp"

#include <functional>
#include "Event.hpp"
#include <map>
#include "../Collection/List.hpp"
#include <shared_mutex>

namespace clockwork::__internal::types {

	/** The event emitter */
	__interface EventEmitter { };

	/**
	 * A stateful event emitter.
	 *
	 * @author Julius
	 * @date 18.01.2020
	 */
	template <class MtxTy_, class ELckTy_ = _STD unique_lock<MtxTy_>, class ULckTy_ = _STD unique_lock<MtxTy_>>
	class StatefulEventEmitterBase :
		public EventEmitter {
	public:

		/** Type of the function */
		using function_type = _STD function<void(Event&)>;

		/**
		 * Register new callback function to mutable event
		 *
		 * @tparam Ty Type of the const event.
		 * @tparam check for convertible type to event
		 * @param event_ The event.
		 * @param [in,out] fnc_ The function.
		 */
		template <typename Ty, typename = _STD enable_if_t<_STD is_convertible_v<Ty, Event>>>
		void on(const string& name_, _STD function<void(Ty&)>&& fnc_) {
			ULckTy_ lck(_mtx);
			_events[name_].emplace_back(_STD forward(fnc_));
		}

		// TODO: Remove Event listener
		template <typename Ty, typename = _STD enable_if_t<_STD is_convertible_v<Ty, Event>>>
		void remove(const string& name_, _STD function<void(Ty&)>&& fnc_) { }

		/**
		 * Invoke listener chain with possible mutation of the given event
		 *
		 * @tparam Ty Type of the const event.
		 * @tparam check for convertible type to event
		 * @param name_ The name.
		 * @param [in,out] event_ The event.
		 */
		template <typename Ty, typename = _STD enable_if_t<_STD is_convertible_v<Ty, Event>>>
		void emit(const string& name_, Ty& event_) {
			ELckTy_ lck(_mtx);

			vector<Ty>& listeners = _events[name_];
			for (auto& entry : listeners) {
				(entry)(event_);

				if (event_.canceled()) {
					break;
				}
			}
		}

	protected:
		/** The events */
		std::map<string, vector<function_type>> _events;

	private:
		MtxTy_ _mtx;
	};

	/**
	 * The stateless event emitter base.
	 *
	 * @author Julius
	 * @date 19.01.2020
	 *
	 * @tparam MtxTy_ Type of the mutex to use.
	 * @tparam ELckTy_ Type of the emitting lock to use.
	 * @tparam ULckTy_ Type of the writing lock to use.
	 */
	template <class MtxTy_, class ELckTy_ = _STD unique_lock<MtxTy_>, class ULckTy_ = _STD unique_lock<MtxTy_>>
	class StatelessEventEmitterBase :
		public EventEmitter {
	public:

		/** Type of the function */
		using function_type = _STD function<void(const Event&)>;

		/**
		 * Register new callback function to immutable event
		 *
		 * @tparam Ty Type of the event.
		 * @tparam check for convertible type to event
		 * @param  name_ The name.
		 * @param  fnc_ The function.
		 */
		template <typename Ty, typename = _STD enable_if_t<_STD is_convertible<Ty, Event>>>
		void on(const string& name_, _STD function<void(const Ty&)>&& fnc_) {
			ULckTy_ lck(_mtx);
			_events[name_].emplace_back(_STD forward(fnc_));
		}

		// TODO: Remove Event listener
		template <typename Ty, typename = _STD enable_if_t<_STD is_convertible<Ty, Event>>>
		void remove(const string& name_, _STD function<void(const Ty&)>&& fnc_) { }

		/**
		 * Invoke listener chain without mutating the given event
		 *
		 * @tparam Ty Type of the event.
		 * @tparam check for convertible type to event
		 * @param  name_ The name.
		 * @param  event_ The event.
		 */
		template <typename Ty, typename = _STD enable_if_t<_STD is_convertible<Ty, Event>>>
		void emit(const string& name_, const Ty& event_) {
			ELckTy_ lck(_mtx);

			vector<Ty>& listeners = _events[name_];
			for (auto& entry : listeners) {
				(entry)(event_);
			}
		}

	protected:
		/** The events */
		std::map<string, vector<function_type>> _events;

	private:
		MtxTy_ _mtx;
	};

	/** Defines an alias representing the stateful event emitter */
	typedef StatefulEventEmitterBase<_STD recursive_mutex> StatefulEventEmitter;

	/** Defines an alias representing the stateless event emitter */
	typedef StatelessEventEmitterBase<_STD recursive_mutex> StatelessEventEmitter;
}
