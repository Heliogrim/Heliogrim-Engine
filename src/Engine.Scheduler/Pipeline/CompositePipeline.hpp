#pragma once

#include <Engine.Common/Sal.hpp>
#include <Engine.Common/Collection/DenseMap.hpp>

#include "CompositeDispatcher.hpp"
#include "StagePipeline.hpp"
#include "Composite/CompositeStage.hpp"

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

	public:
		[[nodiscard]] const non_owning_rptr<class Schedule> getSchedule() const noexcept;

	private:
		/**
		 * Registered unique schedule stages
		 */
		DenseMap<string, uptr<Stage>> _stages;
		Vector<uptr<PipelineStage>> _pipelineStages;

	public:
		[[nodiscard]] const non_owning_rptr<const Stage> registerStage(mref<uptr<PipelineStage>> stage_) override;

		[[nodiscard]] const non_owning_rptr<const Stage> getStage(cref<string> identifier_) const noexcept override;

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
		 * Resolved working stages
		 */
		Vector<uptr<CompositeStage>> _compositeStages;
		u64 _stageHead;
		u64 _stageTail;

	private:
		Vector<uptr<CompositeSlot>> _slots;

	public:
		void dispatch(const non_owning_rptr<CompositeSlot> slot_);

		void complete(const non_owning_rptr<CompositeSlot> slot_);

	public:
		void pushTask(
			const non_owning_rptr<const Stage> stage_,
			mref<task_handle_type> task_
		) override;

	private:
		/**
		 * Composite Stage resolving and optimization
		 */
		using compact_stage_set = CompactSet<non_owning_rptr<const Stage>>;
		using compact_stage_dep_set = CompactSet<struct StageDependency>;
		using ordered_stage_list = Vector<non_owning_rptr<const Stage>>;

	private:
		void collectDependencies(_Inout_ ref<compact_stage_dep_set> collection_) const;

		void inverseDependencies(
			_In_ cref<compact_stage_dep_set> dependencies_,
			_Inout_ ref<compact_stage_dep_set> inverse_
		);

		void collectOrders(_Inout_ ref<Vector<ptr<const ordered_stage_list>>> collection_) const;

		void transformOrdersToDependencies(
			_In_ cref<Vector<ptr<const ordered_stage_list>>> orders_,
			_Inout_ ref<compact_stage_dep_set> dependencies_
		) const;

		bool defineCompositeStage(
			s64 where_,
			const non_owning_rptr<const Stage> stage_,
			cref<compact_stage_dep_set> dependencies_
		);

		[[nodiscard]] non_owning_rptr<CompositeStage> getCompositeStage(const ptr<const Stage> stage_) const;

		/**/

		void optimizeDependencies();

		/**/

		bool resolveCanInsertStage(
			const non_owning_rptr<const Stage> stage_,
			cref<compact_stage_dep_set> stageDependencies_,
			cref<compact_stage_dep_set> invStageDependencies_,
			cref<Vector<ptr<const ordered_stage_list>>> internalOrders_,
			cref<compact_stage_set> stated_
		);

	protected:
		void resolve();

		void prepareSlots();
	};
}
