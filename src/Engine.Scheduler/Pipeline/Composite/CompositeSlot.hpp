#pragma once

#include <Engine.Async/Primitive/SpinLock.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Collection/Vector.hpp>

#include "../../Task/TaskCtrl.hpp"
#include "../../Task/TaskMask.hpp"
#include "../Stage/StageDispatcher.hpp"

namespace hg::engine::scheduler {
	class CompositeSlot :
		public task::TaskCtrl {
	public:
		using this_type = CompositeSlot;

	public:
		CompositeSlot(
			const non_owning_rptr<class CompositePipeline> pipeline_,
			const non_owning_rptr<class Schedule> schedule_
		);

		~CompositeSlot() override;

	private:
		non_owning_rptr<class Schedule> _schedule;
		non_owning_rptr<class CompositePipeline> _pipeline;

		u16 _node;
		task::TaskMask _mask;

	public:
		[[nodiscard]] const non_owning_rptr<class Schedule> getSchedule() const noexcept;

		[[nodiscard]] u16 getNode() const noexcept;

		void setNode(u16 node_) noexcept;

		void setMask(task::TaskMask mask_) noexcept;

	private:
		mutable concurrent::SpinMutex _mtx;
		Vector<non_owning_rptr<const task::TaskDelegate>> _staticTasks;
		Vector<non_owning_rptr<const task::TaskDelegate>> _dynamicTasks;

	public:
		void staticEnqueue(mref<non_owning_rptr<const task::TaskDelegate>> task_);

		void dynamicEnqueue(mref<non_owning_rptr<const task::TaskDelegate>> task_);

	public:
		[[nodiscard]] StaticStageDispatcher getStaticDispatcher() const noexcept;

		[[nodiscard]] DynamicStageDispatcher getDynamicDispatcher() const noexcept;

	private:
		std::atomic_uint_fast16_t _signals;

	public:
		void onComplete(const non_owning_rptr<const task::TaskDelegate> task_) override;

	public:
		void submit();
	};
}
