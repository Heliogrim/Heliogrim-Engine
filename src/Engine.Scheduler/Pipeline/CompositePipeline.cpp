#include "CompositePipeline.hpp"

#include <algorithm>
#include <ranges>
#include <Engine.Common/Make.hpp>
#include <Engine.Logging/Logger.hpp>
#include <Engine.Pedantic/Clone/Clone.hpp>

#include "../Fiber/Fiber.hpp"
#include "../Helper/Wait.hpp"
#include "../Process/Schedule.hpp"
#include "Composite/CompositeSlot.hpp"
#include "Composite/StageDependency.hpp"
#include "Stage/PipelineStage.hpp"

using namespace hg::engine::scheduler;
using namespace hg;

CompositePipeline::CompositePipeline(const non_owning_rptr<Schedule> schedule_) :
	_schedule(schedule_),
	_stopping(),
	_stageHead(0),
	_stageTail(0) {}

CompositePipeline::~CompositePipeline() = default;

void CompositePipeline::setup() {

	for (const auto& pipeline : _pipelines) {
		pipeline->mount(*static_cast<ptr<StageRegister>>(this));
	}

	/**/

	resolve();
}

void CompositePipeline::start() {

	for (const auto& stage : _compositeStages) {
		for (const auto& dependency : stage->dependencies) {
			const_cast<CompositeDependency&>(dependency).requiredGeneration = 1uLL;
		}
	}

	/* Find entry point */

	const auto& firstStage = _compositeStages.front();
	for (const auto& dependency : firstStage->dependencies) {
		const_cast<ref<CompositeDependency>>(dependency).requiredGeneration = 0uLL;
	}

	assert(firstStage->ready());

	/**/

	prepareSlots();

	/**/

	::hg::assertrt(not _stopping.test());
	dispatch(_slots[firstStage->stage->getSlot()].get());
}

void CompositePipeline::stop() {
	::hg::assertrt(not _stopping.test());

	_stopping.test_and_set();
	// TODO: waitOnAtomic(_stopping, true);
	while (_stopping.test(std::memory_order::relaxed) == true) {
		fiber::self::yield();
	}
	if (_stopping.test()) {
		::hg::panic();
	}
}

void CompositePipeline::destroy() {

	for (const auto& pipeline : _pipelines) {
		pipeline->dismount(*static_cast<ptr<StageRegister>>(this));
	}

}

const non_owning_rptr<Schedule> CompositePipeline::getSchedule() const noexcept {
	return _schedule;
}

const non_owning_rptr<const Stage> CompositePipeline::registerStage(mref<uptr<PipelineStage>> stage_) {

	const auto& identifier = stage_->stageIdentifier();
	_pipelineStages.emplace_back(std::move(stage_));

	/**/

	const auto where = _stages.find(identifier.value);
	if (where != _stages.end()) {

		auto* const stage = where->second.get();
		stage->incRef();

		return stage;
	}

	/**/

	auto slot = make_uptr<CompositeSlot>(this, _schedule, nullptr);

	const auto slotIter = _slots.insert(_slots.end(), std::move(slot));
	const auto slotBegin = _slots.begin();
	const auto slotIdx = std::distance(slotBegin, slotIter);

	/**/

	::hg::assertrt(0LL <= slotIdx && slotIdx <= 127LL);

	const auto result = _stages.insert_or_assign(
		identifier.value,
		make_uptr<Stage>(clone(identifier.value), static_cast<s8>(slotIdx))
	);

	return result.first->second.get();
}

const non_owning_rptr<const Stage> CompositePipeline::getStage(cref<string> identifier_) const noexcept {
	const auto where = _stages.find(identifier_);
	return where != _stages.end() ? where->second.get() : nullptr;
}

void CompositePipeline::removeStage(cref<string> identifier_) {

	const auto where = _stages.find(identifier_);
	if (where == _stages.end()) {
		return;
	}

	auto* const stage = where->second.get();
	if (stage->decRef()) {

		const auto pss = std::ranges::remove(
			_pipelineStages,
			stage->getIdentifier(),
			[](const auto& entry_) {
				return entry_->stageIdentifier().value;
			}
		);
		_pipelineStages.erase(pss.begin(), pss.end());

		_stages.erase(where);
	}
}

void CompositePipeline::removeStage(mref<non_owning_rptr<Stage>> stage_) {

	decltype(_stages)::iterator where = _stages.begin();
	while (where != _stages.end() && where->second.get() != stage_) {
		++where;
	}

	/**/

	if (where == _stages.end()) {
		return;
	}

	/**/

	if (where->second->decRef()) {

		const auto pss = std::ranges::remove(
			_pipelineStages,
			where->second->getIdentifier(),
			[](const auto& entry_) {
				return entry_->stageIdentifier().value;
			}
		);
		_pipelineStages.erase(pss.begin(), pss.end());

		_stages.erase(where);
	}
}

const non_owning_rptr<const StagePipeline> CompositePipeline::addPipeline(mref<uptr<StagePipeline>> pipeline_) {
	const auto* const result = pipeline_.get();
	_pipelines.push_back(std::move(pipeline_));
	return result;
}

void CompositePipeline::removePipeline(const non_owning_rptr<StagePipeline> pipeline_) {
	auto where = std::ranges::remove(
		_pipelines,
		pipeline_,
		[](cref<uptr<StagePipeline>> entry_) {
			return entry_.get();
		}
	);

	_pipelines.erase(where.begin(), where.end());
}

void CompositePipeline::dispatch(const non_owning_rptr<CompositeSlot> slot_) {

	// TODO: increment generations of dependencies
	// TODO: bump head/tail helper
	// TODO: push stage tasks to schedule

	const auto& stage = slot_->getCompositeStage();
	for (const auto& dependency : stage->dependencies) {
		++const_cast<ref<CompositeDependency>>(dependency).requiredGeneration;
	}

	/**/

	const auto dispatcher = slot_->getDynamicDispatcher();
	for (const auto* const binding : stage->pipelineStages) {
		const_cast<ptr<PipelineStage>>(binding)->dynamicDispatch(dispatcher);
	}

	/**/

	slot_->submit();
}

void CompositePipeline::complete(const non_owning_rptr<CompositeSlot> slot_) {

	auto* const stage = slot_->getCompositeStage();
	auto compIter = std::ranges::find(
		_compositeStages,
		stage,
		[](cref<uptr<CompositeStage>> entry_) {
			return entry_.get();
		}
	);

	/**/

	assert(compIter != _compositeStages.end());
	++stage->generation;

	/**/

	for (auto iter = compIter + 1; iter != _compositeStages.end(); ++iter) {

		auto* const nextStage = iter->get();
		if (nextStage->ready()) {
			dispatch(_slots[nextStage->stage->getSlot()].get());
		}

		// TODO: Vector to RingBuffer behaviour
	}

	/**/

	// TODO: Rewrite ~ Just a fast forward quickfix
	if (++compIter == _compositeStages.end()) {
		if (not _stopping.test()) {
			assert(_compositeStages.front()->ready());
			dispatch(_slots[_compositeStages.front()->stage->getSlot()].get());

		} else [[unlikely]] {
			_stopping.clear();
		}
	}

	/**/

	// TODO: Change location for priority raising
	_schedule->tick();
}

void CompositePipeline::pushTask(
	const non_owning_rptr<const Stage> stage_,
	mref<task_handle_type> task_
) {
	//_slots[stage_->getSlot()]->dynamicEnqueue(std::move(task_));
	IM_CORE_WARN("Tried to push a task dynamically to a stage slot. (Not Implemented)");
}

void CompositePipeline::collectDependencies(ref<compact_stage_dep_set> collection_) const {
	for (const auto& pipeline : _pipelines) {
		pipeline->declareDependencies(*this, collection_);
	}
}

void CompositePipeline::inverseDependencies(
	cref<compact_stage_dep_set> dependencies_,
	ref<compact_stage_dep_set> inverse_
) {
	for (const auto& dependency : dependencies_) {

		for (const auto* const requirement : dependency.required) {

			auto invIter = std::ranges::find(
				inverse_,
				requirement,
				[](cref<StageDependency> entry_) {
					return entry_.stage;
				}
			);

			if (invIter == inverse_.end()) {

				const auto result = inverse_.insert(
					StageDependency {
						{},
						nullptr,
						requirement
					}
				);

				invIter = result.first;
			}

			/**/

			const_cast<ref<StageDependency>>(*invIter).required.insert(dependency.stage);
		}
	}
}

void CompositePipeline::collectOrders(ref<Vector<ptr<const Vector<non_owning_rptr<const Stage>>>>> collection_) const {
	for (const auto& pipeline : _pipelines) {
		const auto& ordered = pipeline->orderedStages();
		collection_.push_back(&ordered);
	}
}

void CompositePipeline::transformOrdersToDependencies(
	cref<Vector<ptr<const ordered_stage_list>>> orders_,
	ref<compact_stage_dep_set> dependencies_
) const {

	for (const auto* const order : orders_) {

		auto head = order->begin();
		auto tail = order->begin();

		if (head == order->end()) {
			continue;
		}

		/**/

		++head;
		while (head != order->end()) {

			StageDependency dep { { *tail }, nullptr, *head };
			dependencies_.insert(std::move(dep));

			++head;
			++tail;
		}

	}
}

bool CompositePipeline::defineCompositeStage(
	s64 where_,
	const ptr<const Stage> stage_,
	cref<compact_stage_dep_set> dependencies_
) {

	CompactArray<non_owning_rptr<const PipelineStage>> pipelineStages {};
	for (const auto& entry : _pipelineStages) {
		if (entry->stageIdentifier().value == stage_->getIdentifier()) {
			pipelineStages.push_back(entry.get());
		}
	}

	auto cs = make_uptr<CompositeStage>(
		CompositeStage {
			stage_,
			std::move(pipelineStages),
			0uLL,
			CompactSet<CompositeDependency> {}
		}
	);

	/* Link composite stage to slot */

	_slots[stage_->getSlot()]->setCompositeStage(cs.get());

	/**/

	for (const auto& dependency : dependencies_) {

		if (dependency.stage != stage_) {
			continue;
		}

		/**/

		for (const auto* const requirement : dependency.required) {

			auto* const compStage = getCompositeStage(requirement);
			if (compStage == nullptr) {
				throw std::runtime_error("");
			}

			cs->dependencies.insert(CompositeDependency { compStage, 0uLL });
		}
	}

	/**/

	if (where_ >= 0) {
		_compositeStages.insert(_compositeStages.begin() + where_, std::move(cs));
	} else {
		_compositeStages.push_back(std::move(cs));
	}

	return true;
}

non_owning_rptr<CompositeStage> CompositePipeline::getCompositeStage(const ptr<const Stage> stage_) const {

	const auto compIter = std::ranges::find(
		_compositeStages,
		stage_,
		[](cref<uptr<CompositeStage>> compStage_) {
			return compStage_->stage;
		}
	);

	return compIter != _compositeStages.end() ? (compIter->get()) : nullptr;
}

void CompositePipeline::optimizeDependencies() {
	// TODO: Check whether we can reduce the number of direct requirements per composite stage
	// TODO: This will reduce the required compute power to progress further while running the pipeline
}

bool CompositePipeline::resolveCanInsertStage(
	const non_owning_rptr<const Stage> stage_,
	cref<compact_stage_dep_set> stageDependencies_,
	cref<compact_stage_dep_set> invStageDependencies_,
	cref<Vector<ptr<const Vector<non_owning_rptr<const Stage>>>>> internalOrders_,
	cref<compact_stage_set> stated_
) {
	if (stated_.contains(stage_)) {
		return false;
	}

	/* Check direct dependencies */

	for (const auto& entry : stageDependencies_) {

		if (entry.stage != stage_) {
			continue;
		}

		for (const auto& required : entry.required) {
			if (not stated_.contains(required)) {
				return false;
			}
		}
	}

	/* Check inverse dependencies*/

	for (const auto& entry : invStageDependencies_) {

		if (entry.stage != stage_) {
			continue;
		}

		// Attention: We check against inverse requirements ( so they should not be present )
		for (const auto& required : entry.required) {
			if (stated_.contains(required)) {
				return false;
			}
		}
	}

	/* Check internal orders */

	for (const auto* const order : internalOrders_) {

		const auto stageMark = std::ranges::find(*order, stage_);
		if (stageMark == order->end()) {
			continue;
		}

		/**/

		for (auto orderIter = order->begin(); orderIter != stageMark; ++orderIter) {
			if (not stated_.contains(*orderIter)) {
				return false;
			}
		}

		for (auto orderIter = stageMark; orderIter != order->end(); ++orderIter) {
			if (stated_.contains(*orderIter)) {
				return false;
			}
		}

	}

	/**/

	return true;
}

void CompositePipeline::resolve() {

	/* Collect all defined stages */

	CompactSet<ptr<const Stage>> stages {};
	stages.reserve(_stages.size());

	for (const auto& entry : _stages) {
		stages.insert(entry.second.get());
	}

	/* Prepare working sets */

	CompactSet<ptr<const Stage>> sorted {};

	compact_stage_dep_set dependencies {};
	collectDependencies(dependencies);

	Vector<ptr<const Vector<non_owning_rptr<const Stage>>>> internalOrders {};
	collectOrders(internalOrders);
	transformOrdersToDependencies(internalOrders, dependencies);

	compact_stage_dep_set invDependencies {};
	inverseDependencies(dependencies, invDependencies);

	/**/

	const u64 maxIteration = stages.size() + 1uLL;
	u64 iteration = 0;
	while (stages.size() != sorted.size() && iteration <= maxIteration) {

		for (const auto* const stage : stages) {

			if (sorted.contains(stage)) {
				continue;
			}

			/**/

			compact_stage_set forward {};
			for (u64 compIdx = 0; compIdx < _compositeStages.size();) {

				auto* const compStage = _compositeStages[compIdx].get();
				if (forward.contains(compStage->stage)) {
					++compIdx;
					continue;
				}

				forward.insert(compStage->stage);

				/**/

				const auto* const predecessor = _compositeStages[compIdx]->stage;
				while (compIdx < _compositeStages.size() && _compositeStages[compIdx]->stage == predecessor) {
					++compIdx;
				}

				/**/

				if (resolveCanInsertStage(stage, dependencies, invDependencies, internalOrders, forward)) {
					defineCompositeStage(compIdx, stage, dependencies);
					sorted.insert(stage);
					break;
				}
			}

			/**/

			if (_compositeStages.empty()) {

				forward.clear();
				if (resolveCanInsertStage(stage, dependencies, invDependencies, internalOrders, forward)) {
					defineCompositeStage(-1, stage, dependencies);
					sorted.insert(stage);
				}
			}
		}

		++iteration;
	}

	/**/

	if (sorted.size() != stages.size()) {
		IM_CORE_ERROR("Failed to resolve scheduling stages.");
		throw std::runtime_error("");
	}

	/* Optimize Stage Order */

	/* Optimize I :: Reduce Forward Staging Time */

	u64 compIdx = 0;
	const u64 compStageSize = _compositeStages.size();

	while (compIdx < compStageSize) {

		const auto* const stage = _compositeStages[compIdx]->stage;
		const u64 beginCompStage = compIdx;
		while (compIdx < compStageSize && _compositeStages[compIdx]->stage == stage) {
			++compIdx;
		}
		const u64 endCompStage = compIdx;

		/**/

		CompactSet<ptr<const Stage>> forward {};
		for (u64 idx = 0; idx < beginCompStage; ++idx) {
			forward.insert(_compositeStages[idx]->stage);
		}

		/* Search forward insert position until we can't insert anymore */

		u64 forwardCompIdx = endCompStage;
		while (
			resolveCanInsertStage(stage, dependencies, invDependencies, internalOrders, forward) &&
			forwardCompIdx < compStageSize
		) {

			const auto* const nextForwardStage = _compositeStages[forwardCompIdx]->stage;
			while (forwardCompIdx < compStageSize && _compositeStages[forwardCompIdx]->stage == nextForwardStage) {
				++forwardCompIdx;
			}

			forward.insert(nextForwardStage);
		}

		/* If we found the break spot, move one sequence back (begin of prev sequence) */

		if (forwardCompIdx != endCompStage) {

			const auto* const breakStage = _compositeStages[forwardCompIdx - 1]->stage;
			while (forwardCompIdx > endCompStage && _compositeStages[forwardCompIdx - 1]->stage == breakStage) {
				--forwardCompIdx;
			}

			if (forwardCompIdx == endCompStage) {
				continue;
			}

			/* Calculate changed pipeline staging distance */

			s64 changeDistance = 0;
			for (const auto& pipeline : _pipelines) {

				const auto* const begin = pipeline->beginStage();
				const auto* const end = pipeline->endStage();

				/* Prevent failure due to single stage pipelines ~ no distance measure */
				if (begin == end) {
					continue;
				}

				/* Check whether current stage effects outer distance of the pipeline */
				if (stage != begin && stage != end) {
					continue;
				}

				auto beginIter = std::ranges::find(
					_compositeStages,
					begin,
					[](const auto& entry_) {
						return entry_->stage;
					}
				);

				auto endIter = std::ranges::find(
					_compositeStages,
					end,
					[](const auto& entry_) {
						return entry_->stage;
					}
				);

				u64 prevDist = 0;
				for (auto tail = beginIter, head = beginIter + 1; head != endIter; ++tail, ++head) {
					if ((*tail)->stage != (*head)->stage) {
						++prevDist;
					}
				}

				/**/

				if (stage == begin) {
					beginIter = _compositeStages.begin() + forwardCompIdx;
				} else if (stage == end) {
					endIter = _compositeStages.begin() + forwardCompIdx;
				}

				/**/

				u64 nextDist = 0;

				if (beginIter >= endIter) {
					nextDist = 0;

				} else {

					for (auto tail = beginIter, head = beginIter + 1; head != endIter; ++tail, ++head) {
						if ((*tail)->stage != (*head)->stage) {
							++nextDist;
						}
					}

				}

				/**/

				changeDistance += (prevDist - nextDist);
			}

			/**/

			if (changeDistance <= 0) {
				continue;
			}

			/* Move sequence to latest spot */

			for (u64 fromIdx = endCompStage; fromIdx > beginCompStage; --fromIdx) {
				_compositeStages.insert(
					_compositeStages.begin() + forwardCompIdx,
					std::move(_compositeStages[fromIdx - 1])
				);
			}

			const auto beginErase = _compositeStages.begin() + beginCompStage;
			const auto endErase = _compositeStages.begin() + endCompStage;

			_compositeStages.erase(beginErase, endErase);

		}

	}

	/* Optimize II :: Reduce Backward Staging Time */

	/**/

	invDependencies.clear();
	dependencies.clear();
}

void CompositePipeline::prepareSlots() {

	/* Resolve stages with static dispatcher */
	for (const auto& slot : _slots) {

		const auto* const stage = slot->getCompositeStage();
		const auto dispatcher = slot->getStaticDispatcher();

		for (const auto* const binding : stage->pipelineStages) {
			const_cast<ptr<PipelineStage>>(binding)->staticDispatch(dispatcher);
		}
	}

}
