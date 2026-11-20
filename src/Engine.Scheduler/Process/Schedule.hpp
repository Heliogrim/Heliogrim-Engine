#pragma once

#include <Engine.Common/Sal.hpp>

#include "../Lane.hpp"
#include "../LanePolicyConfig.hpp"
#include "ScheduleWorkSet.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::scheduler {
	struct BoostCell;
}

namespace hg::engine::scheduler {
	inline constexpr u64 laneCount = 3uLL;
	static_assert(laneCount == static_cast<u64>(Lane::eInherit), "one work set per concrete lane");

	inline constexpr u32 reserveWindowFrames = 8uL;

	/** A demand reserve not rolled for this long counts as the minimum (the pipeline is stalled or stopped). */
	inline constexpr u32 reserveStaleMillis = 250uL;

	/** Park timeout meaning no deadline (INFINITE on WIN32). */
	inline constexpr u32 parkForever = ~0uL;

	class Schedule {
	private:
		friend struct ScheduleTestAccess;

	public:
		using this_type = Schedule;
		using time_point = std::chrono::steady_clock::time_point;

	public:
		Schedule();

		~Schedule();

	public:
		void setup(const u32 workers_, ref<const LanePolicyConfig> config_);

		void tidy();

	private:
		alignas(64) std::atomic<u32> _pushEpoch { 0uL };
		alignas(64) Array<std::atomic<u64>, laneCount> _depth {};

		/** Last lower-lane start per lower lane (Interactive, Background), in steady_clock ticks; raised only forward. */
		alignas(64) Array<std::atomic<time_point::rep>, 2uLL> _lastStart {};

		alignas(64) std::atomic<u32> _lowerRunning { 0uL };
		alignas(64) std::atomic<u32> _frameRunning { 0uL };
		std::atomic<u32> _framePeak { 0uL };
		std::atomic<u32> _reserve { 0uL };

		/** wakeOne calls so far; read by tests and benchmarks only. */
		alignas(64) std::atomic<u64> _wakeCalls { 0uLL };

		/** Time of the last roll (or setup), in steady_clock ticks. */
		std::atomic<time_point::rep> _lastRoll { 0 };

		std::atomic<u64> _frameWaitMax { 0uLL };
		Array<std::atomic<u64>, laneCount> _overruns {};
		Array<std::atomic<u64>, laneCount> _maxRun {};
		Array<std::atomic<bool>, laneCount> _overrunLogged {};

	private:
		u32 _workers { 1uL };
		LanePolicyConfig _config {};

		/** Written by rollFrame only. */
		Array<u32, reserveWindowFrames> _peaks {};
		u32 _peakIdx { 0uL };

	private:
		alignas(64) Array<uptr<ScheduleWorkSet>, laneCount> _workSets;

	public:
		[[nodiscard]] u32 epoch() const noexcept;

		[[nodiscard]] u64 queued() const noexcept;

		[[nodiscard]] u64 queued(const Lane lane_) const noexcept;

		void wakeOne() noexcept;

		[[nodiscard]] u64 wakeCalls() const noexcept;

		void wakeAll() noexcept;

		/**
		 * Blocks while the push epoch equals seen_, at most timeoutMillis_; may return spuriously.
		 */
		void waitForWork(u32 seen_, const u32 timeoutMillis_ = parkForever) noexcept;

	public:
		/**
		 * Queues task_ at its lane. With wake_, a non-worker push wakes one worker and a worker push wakes one only
		 * when it fills an empty lane; the cascade on pop spreads the rest. wake_ false never wakes.
		 */
		[[nodiscard]] bool push(_In_ const non_owning_rptr<const task::TaskDelegate> task_, const bool wake_ = true);

		/**
		 * Moves task_ from a lane below to_ into to_; false if it is not queued below to_ (running, gone, or already
		 * at to_ or higher, or queued under another cell). Never lowers. Matches only an entry pushed by pushBoostable
		 * with cell_, so a reused address is never moved; the re-push is pushBoostable, so a racing boost is not lost.
		 * cell_ is non-null and the caller must hold a counted reference to it for the whole call.
		 */
		[[nodiscard]] bool promote(
			_In_ const non_owning_rptr<const task::TaskDelegate> task_,
			const Lane to_,
			_In_ const ptr<BoostCell> cell_
		);

		/**
		 * Pushes task_ at the higher of its lane and the lane of cell_, then promotes it if the cell rose meanwhile;
		 * a null cell is a plain push. Only the owner of task_ (the worker re-pushing it, or the unparker) calls this.
		 * cell_ must be alive on entry (task_ or the caller keeps it); a counted reference keeps it alive past the push.
		 */
		[[nodiscard]] bool pushBoostable(
			_In_ const non_owning_rptr<const task::TaskDelegate> task_,
			_In_opt_ ptr<BoostCell> cell_,
			const bool wake_ = true
		);

		/**
		 * True if a pop at now_ could take work: the frame lane is queued, or a lower lane is queued and its floor is
		 * due or the reserve leaves it a worker. A racy hint; a false positive only costs a wake.
		 */
		[[nodiscard]] bool takeable(const time_point now_) const noexcept;

		/** Called by a worker after a pop: wakes two peers if more work is takeable. */
		void cascade(const time_point now_) noexcept;

		/** Wakes for pushed_ quiet pushes: all of them from a non-worker, at most one from a worker. */
		void wakeAfterSubmit(const u32 pushed_) noexcept;

		[[nodiscard]] bool pop(
			_In_ const time_point now_,
			_Inout_ ref<non_owning_rptr<const task::TaskDelegate>> task_,
			_Out_ ref<Lane> lane_
		);

		/**
		 * Park bound for a worker whose pop failed: until the earliest floor of a lower lane with queued work falls due,
		 * else parkForever. Returns 0 while a Frame push is in flight (Frame depth > 0).
		 */
		[[nodiscard]] u32 parkTimeoutMillis(const time_point now_) const noexcept;

	public:
		/**
		 * Called by the worker when a fiber of lane_ returns to it (finished, yielded or parked).
		 */
		void leave(const Lane lane_) noexcept;

		/** Called by the worker when a fiber of lane_ returns after micros_ of running. */
		void noteRun(const Lane lane_, const u64 micros_) noexcept;

		[[nodiscard]] u32 sliceMicros() const noexcept;

		/** Runs of a lower lane that took longer than twice the slice between launch and return. */
		[[nodiscard]] u64 overruns(const Lane lane_) const noexcept;

		[[nodiscard]] u64 maxRunMicros(const Lane lane_) const noexcept;

	public:
		/**
		 * Closes a frame for the demand-sized reserve; one caller at a time.
		 */
		void rollFrame() noexcept;

		/**
		 * Forgets the Frame demand once the pipeline has stopped; not concurrent with rollFrame.
		 */
		void resetReserve() noexcept;

		[[nodiscard]] u32 reserve() const noexcept;

		/** The reserve pop applies at now_: the stored one, or the minimum once a demand reserve went stale. */
		[[nodiscard]] u32 effectiveReserve(const time_point now_) const noexcept;

		/**
		 * Longest Frame wait in microseconds since the last roll.
		 */
		[[nodiscard]] u64 frameWaitMaxMicros() const noexcept;

	private:
		[[nodiscard]] u32 targetReserve() const noexcept;

		void noteFrameWait(const time_point now_, _In_ const non_owning_rptr<const task::TaskDelegate> task_) noexcept;
	};
}
