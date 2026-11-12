#pragma once

#include <atomic>
#include <Engine.Common/Sal.hpp>
#include <Engine.Common/Collection/DenseMap.hpp>

#include "CompositeDispatcher.hpp"
#include "StagePipeline.hpp"
#include "../Waitable/Signal.hpp"
#include "../Graph/TaskGraph.hpp"

namespace hg::engine::scheduler {
	class CompositeSlot;
}

namespace hg::engine::scheduler {
	class CompositePipeline :
		public StageRegister,
		public CompositeDispatcher {
	public:
		using this_type = CompositePipeline;

	public:
		CompositePipeline(const non_owning_rptr<class Schedule> schedule_);

		~CompositePipeline() override;

	public:
		void setup();

		void start();

		void stop();

		void destroy();

	private:
		/**
		 * Schedule
		 */
		non_owning_rptr<class Schedule> _schedule;
		std::atomic_flag _stopping;
		Signal _stopped;

	public:
		[[nodiscard]] const non_owning_rptr<class Schedule> getSchedule() const noexcept;

	private:
		/**
		 * Registered unique schedule stages
		 */
		DenseMap<string, uptr<Stage>> _stages;
		Vector<uptr<PipelineStage>> _pipelineStages;

	public:
		[[nodiscard]] Opt<ref<const Stage>> registerStage(mref<uptr<PipelineStage>> stage_) override;

		[[nodiscard]] Opt<ref<const Stage>> getStage(cref<string> identifier_) const noexcept override;

		void removeStage(cref<string> identifier_) override;

		void removeStage(mref<non_owning_rptr<Stage>> stage_) override;

	private:
		/**
		 * Pipelines to consider
		 */
		Vector<uptr<StagePipeline>> _pipelines;

	public:
		const non_owning_rptr<const StagePipeline> addPipeline(mref<uptr<StagePipeline>> pipeline_);

		void removePipeline(const non_owning_rptr<StagePipeline> pipeline_);

	private:
		/**
		 * Built stage graph and per-frame counters
		 */
		graph::Graph _graph;
		Vector<Vector<ptr<PipelineStage>>> _nodeStages;
		Vector<u16> _slotOfNode;
		uptr<std::atomic<u16>[]> _pending;
		std::atomic<u16> _remaining;
		u32 _maxInFlight;
		std::atomic<u32> _activeCalls;

	private:
		Vector<uptr<CompositeSlot>> _slots;

	public:
		void setMaxInFlight(const u32 count_) noexcept;

		[[nodiscard]] ref<const graph::Graph> graph() const noexcept;

		[[nodiscard]] bool isStopping() const noexcept;

		void complete(const non_owning_rptr<CompositeSlot> slot_);

	public:
		void pushTask(
			const non_owning_rptr<const Stage> stage_,
			mref<task_handle_type> task_
		) override;

	private:
		void startFrame();

		void dispatch(const u16 node_);
	};
}
