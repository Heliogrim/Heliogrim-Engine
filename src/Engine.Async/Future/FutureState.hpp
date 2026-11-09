#pragma once

#include <atomic>
#include <Engine.Common/Types.hpp>
#include <Engine.Common/Variant.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Functional/UniqueFn.hpp>
#include <Engine.Common/Managed/Rc.hpp>
#include <Engine.Scheduler/Task/TaskMask.hpp>
#include <Engine.Scheduler/Waitable/Signal.hpp>

#include "Broken.hpp"

namespace hg {
	struct Pending {};

	template <typename T_>
	struct Resolved {
		T_ value;
	};

	template <>
	struct Resolved<void> {};

	template <typename E_>
	struct Failed {
		Fail<E_> error;
	};

	namespace detail {
		inline constexpr u8 eNone = 0;
		inline constexpr u8 eStored = 1;
		inline constexpr u8 eClaimed = 2;

		template <typename T_, typename E_>
		class FutureState :
			public ArcFromThis<FutureState<T_, E_>> {
		public:
			engine::scheduler::Signal done;
			Variant<Pending, Resolved<T_>, Failed<E_>> slot;
			std::atomic<u8> contState { eNone };
			// The successor's work, stored by then() on this (predecessor) state.
			UniqueFn<void(ref<FutureState>)> cont;
			engine::scheduler::task::TaskMask mask = engine::scheduler::task::TaskMask::eNormal;
		};
	}
}
