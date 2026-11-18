#pragma once

#include <atomic>
#include <Engine.Common/Sal.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Collection/Array.hpp>

#include "ScheduleWorkSet.hpp"
#include "../Lane.hpp"

namespace hg::engine::scheduler {
	inline constexpr u64 laneCount = 3uLL;
	static_assert(laneCount == static_cast<u64>(Lane::eInherit), "one work set per concrete lane");

	class Schedule {
	public:
		using this_type = Schedule;

	public:
		Schedule();

		~Schedule();

	public:
		void setup();

		void tidy();

	private:
		alignas(64) std::atomic<u32> _pushEpoch { 0uL };
		alignas(64) std::atomic<u64> _queued { 0uLL };

	private:
		alignas(64) Array<uptr<ScheduleWorkSet>, laneCount> _workSets;

	public:
		[[nodiscard]] u32 epoch() const noexcept;

		[[nodiscard]] u64 queued() const noexcept;

		void wakeOne() noexcept;

		void wakeAll() noexcept;

		/**
		 * Blocks while the push epoch equals seen_; may return spuriously.
		 */
		void waitForWork(u32 seen_) noexcept;

	public:
		[[nodiscard]] bool push(_In_ const non_owning_rptr<const task::TaskDelegate> task_);

		[[nodiscard]] bool pop(_Inout_ ref<non_owning_rptr<const task::TaskDelegate>> task_);
	};
}
