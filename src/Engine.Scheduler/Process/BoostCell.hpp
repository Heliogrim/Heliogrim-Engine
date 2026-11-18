#pragma once

#include <atomic>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Managed/Rc.hpp>

#include "../Lane.hpp"

namespace hg::engine::scheduler::task {
	class TaskDelegate;
}

namespace hg::engine::scheduler {
	class Schedule;

	/** Guard against a corrupted link chain, not a limit of the design: a walk stops after this many cells. */
	inline constexpr u32 boostMaxHops = 1024uL;

	/**
	 * The link between a producing task and whoever waits for its result (lane policy design 7.1).
	 */
	struct BoostCell :
		public ArcFromThis<BoostCell> {
	public:
		#ifdef _DEBUG
		/** Poisons lane, so a read through a dangling cell fails the lane assert in pushBoostable. */
		~BoostCell();
		#endif

	public:
		/** Only ever raised. */
		std::atomic<Lane> lane { Lane::eBackground };

		/** The producing task while it exists; compared against queue entries, never dereferenced. */
		std::atomic<ptr<const task::TaskDelegate>> task { nullptr };

		/** Spin lock over dependsOn, as ParkQueue locks. */
		mutable std::atomic_flag linkLock {};

		/** What the producer waits for now; guarded by linkLock. */
		Arci<BoostCell> dependsOn { nullptr };

	public:
		/**
		 * Raises lane to lane_ if lane_ is higher; never lowers.
		 */
		void raise(const Lane lane_) noexcept;

		/**
		 * Raises lane to lane_ if lane_ is higher; true if this call changed it.
		 */
		[[nodiscard]] bool tryRaise(const Lane lane_) noexcept;

		[[nodiscard]] Arci<BoostCell> dependency() const noexcept;

		/** nullptr clears the link. */
		void setDependency(Arci<BoostCell> dependency_) noexcept;
	};

	/**
	 * Raises cell_ and what it depends on to lane_ and promotes their queued producers in the calling fiber's
	 * schedule. Outside a fiber (no schedule) the cells are raised and nothing is promoted; a producer then takes
	 * the raised lane at its next push.
	 */
	void boost(Arci<BoostCell> cell_, const Lane lane_);

	/** As boost(cell_, lane_), promoting in schedule_ (nullptr: raise only). */
	void boost(Arci<BoostCell> cell_, const Lane lane_, const ptr<Schedule> schedule_);

	/**
	 * Stores dependency_ as cell_'s link, then reads cell_'s lane and boosts the dependency to it, so a boost that
	 * passed before the link was stored is carried on. Every linker goes through here.
	 */
	void linkDependency(ref<BoostCell> cell_, Arci<BoostCell> dependency_);

	/** As linkDependency(cell_, dependency_), promoting in schedule_ (nullptr: raise only). */
	void linkDependency(ref<BoostCell> cell_, Arci<BoostCell> dependency_, const ptr<Schedule> schedule_);
}
