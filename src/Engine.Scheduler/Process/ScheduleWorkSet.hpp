#pragma once

#include <Engine.Common/Sal.hpp>

#include "../Task/Task.hpp"

namespace hg::engine::scheduler {
	class macro_novtable ScheduleWorkSet {
	public:
		using this_type = ScheduleWorkSet;

	protected:
		ScheduleWorkSet() = default;

	public:
		virtual ~ScheduleWorkSet() = default;

	public:
		[[nodiscard]] virtual bool push(_In_ const non_owning_rptr<const task::TaskDelegate> task_) = 0;

		[[nodiscard]] virtual bool pop(_Inout_ ref<non_owning_rptr<const task::TaskDelegate>> task_) = 0;
	};
}
