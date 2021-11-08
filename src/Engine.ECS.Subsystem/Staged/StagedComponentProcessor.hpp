#pragma once

#include <Engine.Common/Wrapper.hpp>
#include <Engine.Scheduler/Scheduler.hpp>

#include "StagedComponentQueue.hpp"

namespace ember::engine::ecs::subsystem {

	class StagedComponentProcessor {
	public:
		StagedComponentProcessor() noexcept;

		StagedComponentProcessor(mref<StagedComponentProcessor> other_) noexcept;

		StagedComponentProcessor(cref<StagedComponentProcessor>) = delete;

		~StagedComponentProcessor() noexcept;

	public:
		ref<StagedComponentProcessor> operator=(mref<StagedComponentProcessor> other_) noexcept;

		ref<StagedComponentProcessor> operator=(cref<StagedComponentProcessor>) = delete;

	public:
		void tidy() noexcept;

		void schedule(
			_In_ const ptr<scheduler::Scheduler> scheduler_ _In_ const scheduler::ScheduleStage stage_) noexcept;

	private:
		vector<StagedComponentQueue> _queues;

	public:
		[[nodiscard]] _Success_(return != nullptr) ptr<StagedComponentQueue> get(
			_In_ cref<component_type_id> componentTypeId_) const noexcept;

		void push(_In_ cref<component_type_id> componentTypeId_);
	};
}
