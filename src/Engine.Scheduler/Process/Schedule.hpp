#pragma once

#include <atomic>
#include <Engine.Common/Sal.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Collection/Array.hpp>

#include "ScheduleWorkSet.hpp"
#include "../Task/TaskMask.hpp"

namespace hg::engine::scheduler {
	namespace {
		inline constexpr static task::TaskMask priorities[] {
			task::TaskMask::eCritical,
			task::TaskMask::eHigher,
			task::TaskMask::eNormal,
			task::TaskMask::eLower
		};

		inline constexpr static u64 priority_count = sizeof(priorities) / sizeof(task::TaskMask);
	}

	class Schedule {
	public:
		using this_type = Schedule;

	public:
		Schedule();

		~Schedule();

	public:
		void setup();

		void tick();

		void tidy();

	private:
		alignas(64) std::atomic<u32> _pushEpoch { 0uL };
		alignas(64) std::atomic<u64> _queued { 0uLL };

	private:
		alignas(64) Array<uptr<ScheduleWorkSet>, priority_count> _workSets;

	private:
		[[nodiscard]] u64 maskToPriority(const task::TaskMask mask_) const noexcept;

		void transfer(const task::TaskMask src_, const task::TaskMask dst_);

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

		[[nodiscard]] bool pop(
			_In_ const task::TaskMask mask_,
			_Inout_ ref<non_owning_rptr<const task::TaskDelegate>> task_
		);
	};
}
