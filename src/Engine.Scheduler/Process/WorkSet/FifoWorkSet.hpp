#pragma once

#include <Engine.Async/Primitive/SpinLock.hpp>
#include <Engine.Common/Collection/Deque.hpp>

#include "../ScheduleWorkSet.hpp"

namespace hg::engine::scheduler {
	class FifoWorkSet :
		public ScheduleWorkSet {
	public:
		using this_type = FifoWorkSet;

	public:
		FifoWorkSet();

		~FifoWorkSet() override;

	private:
		Deque<non_owning_rptr<const task::TaskDelegate>> _fifo;
		concurrent::UnfairSpinLock _lck;

	public:
		[[nodiscard]] bool push(const non_owning_rptr<const task::TaskDelegate> task_) override;

		[[nodiscard]] bool pop(ref<non_owning_rptr<const task::TaskDelegate>> task_) override;

		[[nodiscard]] bool erase(
			const non_owning_rptr<const task::TaskDelegate> task_,
			const ptr<const BoostCell> cell_
		) override;
	};
}
