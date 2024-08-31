#pragma once

#include <Engine.Common/Cast.hpp>
#include <Engine.Common/Sal.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Functional/Function.hpp>
#include <Engine.Common/Meta/IsAnyOf.hpp>

#include "../../Task/Task.hpp"

namespace hg::engine::scheduler {
	template <typename... TaskTypes_>
	struct StageDispatcher {
	public:
		template <class SelfType_>
		using fnc_type = bound_tuple_fnc<SelfType_, void, mref<non_owning_rptr<const task::TaskDelegate>>>;

		using dispatch_fnc_type = fnc_type<StageDispatcher>;

	public:
		constexpr StageDispatcher() :
			_self(nullptr),
			_fnc(nullptr) {}

		constexpr StageDispatcher(const ptr<void> self_, dispatch_fnc_type fnc_) :
			_self(self_),
			_fnc(fnc_) {}

		template <class SelfType_>
		StageDispatcher(const ptr<SelfType_> self_, fnc_type<SelfType_> fnc_) :
			_self(self_),
			_fnc(void_cast<fnc_type<SelfType_>, dispatch_fnc_type> {}(fnc_)) {}

	private:
		ptr<void> _self;
		dispatch_fnc_type _fnc;

	public:
		template <typename TaskType_>
			requires IsAnyOf<std::remove_const_t<std::remove_pointer_t<TaskType_>>, TaskTypes_...>
		void enqueue(_In_ TaskType_&& task_) const {
			(void)(static_cast<ptr<StageDispatcher>>(_self)->*_fnc)(
				std::forward<non_owning_rptr<const task::TaskDelegate>>(task_)
			);
		}
	};

	/**/

	using StaticStageDispatcher = StageDispatcher<task::RepetitiveTask>;
	using DynamicStageDispatcher = StageDispatcher<task::Task, task::BatchTask>;
}
