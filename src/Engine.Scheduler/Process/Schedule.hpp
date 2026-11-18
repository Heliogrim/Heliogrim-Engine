#pragma once

#include <atomic>
#include <chrono>
#include <Engine.Common/Sal.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Collection/Array.hpp>

#include "ScheduleWorkSet.hpp"
#include "../Lane.hpp"
#include "../LanePolicyConfig.hpp"

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

		[[nodiscard]] u32 reserve() const noexcept;

		/** The reserve pop applies at now_: the stored one, or the minimum once a demand reserve went stale. */
		[[nodiscard]] u32 effectiveReserve(const time_point now_) const noexcept;

		/**
		 * Longest Frame wait in microseconds since the last roll.
		 */
		[[nodiscard]] u64 frameWaitMaxMicros() const noexcept;

		[[nodiscard]] u32 sliceMicros() const noexcept;

		/** Runs of a lower lane that took longer than twice the slice between launch and return. */
		[[nodiscard]] u64 overruns(const Lane lane_) const noexcept;

		[[nodiscard]] u64 maxRunMicros(const Lane lane_) const noexcept;

		/** Called by the worker when a fiber of lane_ returns after micros_ of running. */
		void noteRun(const Lane lane_, const u64 micros_) noexcept;

		void wakeOne() noexcept;

		void wakeAll() noexcept;

		/**
		 * Blocks while the push epoch equals seen_, at most timeoutMillis_; may return spuriously.
		 */
		void waitForWork(u32 seen_, const u32 timeoutMillis_ = parkForever) noexcept;

	public:
		[[nodiscard]] bool push(_In_ const non_owning_rptr<const task::TaskDelegate> task_);

		[[nodiscard]] bool pop(
			_In_ const time_point now_,
			_Inout_ ref<non_owning_rptr<const task::TaskDelegate>> task_,
			_Out_ ref<Lane> lane_
		);

		/**
		 * Park bound for a worker whose pop failed: until the earliest floor of a lower lane with queued work falls due,
		 * else parkForever.
		 */
		[[nodiscard]] u32 parkTimeoutMillis(const time_point now_) const noexcept;

		/**
		 * Called by the worker when a fiber of lane_ returns to it (finished, yielded or parked).
		 */
		void leave(Lane lane_) noexcept;

		/**
		 * Closes a frame for the demand-sized reserve; one caller at a time.
		 */
		void rollFrame() noexcept;

		/**
		 * Forgets the Frame demand once the pipeline has stopped; not concurrent with rollFrame.
		 */
		void resetReserve() noexcept;

	private:
		[[nodiscard]] u32 targetReserve() const noexcept;

		void noteFrameWait(const time_point now_, _In_ const non_owning_rptr<const task::TaskDelegate> task_) noexcept;
	};
}
