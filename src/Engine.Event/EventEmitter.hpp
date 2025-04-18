#pragma once
#include <mutex>
#include <shared_mutex>
#include <type_traits>
#include <unordered_map>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Functional/Function.hpp>

#include "Event.hpp"

namespace hg {
	/** The event emitter */
	class macro_novtable EventEmitter {
	public:
		/**
		 * The data type for the listener handle to remove listener from emitter
		 */
		using handle_type = u64;

	public:
		constexpr virtual ~EventEmitter() noexcept = default;
	};

	template <IsStatefulEvent EventType_>
	struct StatefulEventExecutor {
		void operator()(cref<std::function<void(ref<EventType_>)>> fnc_, ref<EventType_> event_) const {
			fnc_(event_);
		}
	};

	/**
	 * A stateful event emitter.
	 *
	 * @author Julius
	 * @date 18.01.2020
	 */
	template <
		IsStatefulEvent EventType_,
		class MtxType_,
		bool ExternalMtx_ = false,
		typename ExecutorType_ = StatefulEventExecutor<EventType_>,
		class EmitLockType_ = std::unique_lock<std::remove_cvref_t<MtxType_>>,
		class MutateLockType_ = std::unique_lock<std::remove_cvref_t<MtxType_>>>
	class StatefulEventEmitterBase :
		public EventEmitter,
		private ExecutorType_ {
	public:
		using mtx_type = std::conditional_t<ExternalMtx_,
			ref<std::remove_cvref_t<MtxType_>>,
			std::remove_cvref_t<MtxType_>
		>;

	public:
		/**
		 * Type of the callback function to invoke
		 */
		using function_type = std::function<void(ref<EventType_>)>;

	public:
		template <class MtxType = MtxType_> requires (not ExternalMtx_)
		StatefulEventEmitterBase() :
			_lhc(0),
			_listener(),
			_mtx() {}

		template <class MtxType = MtxType_> requires (ExternalMtx_)
		explicit StatefulEventEmitterBase(ref<std::remove_cvref_t<MtxType_>> mtx_) :
			_lhc(0),
			_listener(),
			_mtx(mtx_) {}

		~StatefulEventEmitterBase() override = default;

	public:
		/**
		 * Register a listener function and returns a handle
		 *
		 * @author Julius
		 * @date 07.10.2021
		 *
		 * @param  fnc_ The function.
		 *
		 * @returns An u64.
		 */
		u64 on(mref<std::function<void(ref<EventType_>)>> fnc_) {

			MutateLockType_ lck { _mtx };

			const auto handle { ++_lhc };
			_listener.insert(std::make_pair(handle, std::forward<function_type>(fnc_)));

			return handle;
		}

		/**
		 * Removes a listener by the given handle
		 *
		 * @author Julius
		 * @date 07.10.2021
		 *
		 * @param  handle_ The handle to remove.
		 *
		 * @returns True if it succeeds, false if it fails.
		 */
		bool remove(cref<handle_type> handle_) {

			MutateLockType_ lck { _mtx };

			const auto it = _listener.find(handle_);

			if (it == _listener.end()) {
				return false;
			}

			_listener.erase(it);
			return true;
		}

		void emit(ptr<EventType_> event_) {
			emit(*event_);
		}

		void emit(ref<EventType_> event_) {

			EmitLockType_ lck { _mtx };

			for (const auto& entry : _listener) {
				ExecutorType_::operator()(entry.second, event_);

				if (event_.canceled()) {
					break;
				}
			}
		}

	protected:
		/**
		 * Listener handle count
		 */
		u64 _lhc;

		/**
		 * Listener mapping
		 */
		std::unordered_map<u64, function_type> _listener;

	private:
		mtx_type _mtx;
	};

	template <IsStatelessEvent EventType_>
	struct StatelessEventExecutor {
		void operator()(cref<std::function<void(cref<EventType_>)>> fnc_, cref<EventType_> event_) const {
			fnc_(event_);
		}
	};

	/**
	 * The stateless event emitter base.
	 *
	 * @author Julius
	 * @date 19.01.2020
	 */
	template <
		IsStatelessEvent EventType_,
		class MtxType_,
		bool ExternalMtx_ = false,
		typename ExecutorType_ = StatelessEventExecutor<EventType_>,
		class EmitLockType_ = std::unique_lock<std::remove_cvref_t<MtxType_>>,
		class MutateLockType_ = std::unique_lock<std::remove_cvref_t<MtxType_>>>
	class StatelessEventEmitterBase :
		public EventEmitter,
		private ExecutorType_ {
	public:
		using this_type = StatelessEventEmitterBase<
			EventType_,
			MtxType_,
			ExternalMtx_,
			ExecutorType_,
			EmitLockType_,
			MutateLockType_
		>;

		using mtx_type = std::conditional_t<ExternalMtx_,
			ref<std::remove_cvref_t<MtxType_>>,
			std::remove_cvref_t<MtxType_>
		>;

	public:
		/**
		 * Type of the callback function to invoke
		 */
		using function_type = std::function<void(cref<EventType_>)>;

	public:
		/*
		template <typename MtxType = MtxType_, typename = std::enable_if_t<!ExternalMtx_>>
		StatelessEventEmitterBase() :
		    _lhc(0),
		    _listener(),
		    _mtx() {}
		 */

		/*
		template <typename MtxType = MtxType_, typename = std::enable_if_t<ExternalMtx_>>
		StatelessEventEmitterBase(ref<std::remove_cvref_t<MtxType>> mtx_) :
		    _lhc(0),
		    _listener(),
		    _mtx(mtx_) {}
		 */

		StatelessEventEmitterBase() :
			_lhc(0),
			_listener(),
			_mtx() {}

		explicit StatelessEventEmitterBase(
			std::conditional_t<ExternalMtx_, ref<std::remove_cvref_t<MtxType_>>, std::remove_cvref_t<MtxType_>> mtx_
		) :
			_lhc(0),
			_listener(),
			_mtx(mtx_) {}

		~StatelessEventEmitterBase() override = default;

	public:
		/**
		 * Copy Assignment operator
		 *
		 * @author Julius
		 * @date 12.10.2021
		 *
		 * @param  other_ The other.
		 *
		 * @returns A shallow copy of this.
		 */
		ref<this_type> operator=(cref<this_type> other_) = delete;

		/**
		 * Move Assignment operator
		 *
		 * @author Julius
		 * @date 12.10.2021
		 *
		 * @param  other_ The other.
		 *
		 * @returns A shallow copy of this.
		 */
		ref<this_type> operator=(mref<this_type> other_) noexcept = delete;

	public:
		/**
		 * Register a listener function and returns a handle
		 *
		 * @author Julius
		 * @date 07.10.2021
		 *
		 * @param  fnc_ The function.
		 *
		 * @returns An u64.
		 */
		u64 on(mref<std::function<void(cref<EventType_>)>> fnc_) {

			MutateLockType_ lck { _mtx };

			handle_type handle { ++_lhc };
			_listener.insert(
				std::make_pair<handle_type, function_type>(std::move(handle), std::forward<function_type>(fnc_))
			);

			return handle;
		}

		/**
		 * Removes a listener by the given handle
		 *
		 * @author Julius
		 * @date 07.10.2021
		 *
		 * @param  handle_ The handle to remove.
		 *
		 * @returns True if it succeeds, false if it fails.
		 */
		bool remove(cref<handle_type> handle_) {

			MutateLockType_ lck { _mtx };

			const auto it = _listener.find(handle_);

			if (it == _listener.end()) {
				return false;
			}

			_listener.erase(it);
			return true;
		}

		void emit(ptr<EventType_> event_) {
			emit(*event_);
		}

		void emit(cref<EventType_> event_) {

			EmitLockType_ lck { _mtx };

			for (const auto& entry : _listener) {
				ExecutorType_::operator()(entry.second, event_);
			}
		}

	protected:
		/**
		 * Listener handle count
		 */
		u64 _lhc;

		/**
		 * Listener mapping
		 */
		std::unordered_map<u64, function_type> _listener;

	private:
		mtx_type _mtx;
	};

	/** Defines an alias representing the stateful event emitter */
	template <IsStatefulEvent EventType_>
	using StatefulEventEmitter = StatefulEventEmitterBase<EventType_, std::recursive_mutex>;

	/** Defines an alias representing the stateless event emitter */
	template <IsStatelessEvent EventType_>
	using StatelessEventEmitter = StatelessEventEmitterBase<EventType_, std::recursive_mutex>;
}
