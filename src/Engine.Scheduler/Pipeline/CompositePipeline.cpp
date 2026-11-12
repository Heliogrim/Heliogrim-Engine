#include "CompositePipeline.hpp"

#include <algorithm>
#include <ranges>
#include <Engine.Asserts/Asserts.hpp>
#include <Engine.Common/Make.hpp>
#include <Engine.Logging/Logger.hpp>
#include <Engine.Pedantic/Clone/Clone.hpp>

#include "../Fiber/Fiber.hpp"
#include "../Process/Schedule.hpp"
#include "../Thread/Thread.hpp"
#include "Composite/CompositeSlot.hpp"
#include "Stage/PipelineStage.hpp"

using namespace hg::engine::scheduler;
using namespace hg;

CompositePipeline::CompositePipeline(const non_owning_rptr<Schedule> schedule_) :
	_schedule(schedule_),
	_stopping(),
	_remaining(0u),
	_maxInFlight(0uL),
	_activeCalls(0uL) {}

CompositePipeline::~CompositePipeline() = default;

[[noreturn]] static void failGraph(ref<const graph::GraphError> error_) {

	string stages {};
	for (const auto& stage : error_.stages) {
		stages.append(stage);
		stages.append(" ");
	}

	IM_CORE_ERRORF(
		"Failed to build the stage graph. (kind: {}, stages: {}, key: {})",
		static_cast<u32>(error_.kind),
		stages,
		error_.key.has_value() ? error_.key->id().data : 0uLL
	);
	::hg::assertrt(false);
	std::abort();
}

void CompositePipeline::setup() {

	for (const auto& pipeline : _pipelines) {
		pipeline->mount(*static_cast<ptr<StageRegister>>(this));
	}

	/* Merge every pipeline stage registered under one name into one declaration */

	struct Merged {
		graph::StageDecl decl;
		Vector<graph::AccessKey> reads;
		Vector<graph::AccessKey> writes;
		Vector<string_view> after;
		Vector<string_view> before;
		Vector<ptr<PipelineStage>> stages;
	};

	Vector<Merged> merged {};
	for (const auto& stage : _pipelineStages) {

		const string_view name = stage->stageIdentifier().value;
		const auto decl = stage->decl();

		auto where = std::ranges::find(merged, name, [](ref<const Merged> entry_) { return entry_.decl.name; });
		if (where == merged.end()) {
			merged.emplace_back(Merged { .decl = graph::StageDecl { .name = name, .lane = decl.lane } });
			where = merged.end() - 1;
		}

		if (where->decl.lane != decl.lane) {
			failGraph(graph::GraphError { graph::GraphError::Kind::eLaneMismatch, { string { name } }, None });
		}

		where->reads.insert(where->reads.end(), decl.reads.begin(), decl.reads.end());
		where->writes.insert(where->writes.end(), decl.writes.begin(), decl.writes.end());
		where->after.insert(where->after.end(), decl.after.begin(), decl.after.end());
		where->before.insert(where->before.end(), decl.before.begin(), decl.before.end());
		where->stages.push_back(stage.get());
	}

	Vector<graph::StageDecl> decls {};
	for (auto& entry : merged) {
		entry.decl.reads = entry.reads;
		entry.decl.writes = entry.writes;
		entry.decl.after = entry.after;
		entry.decl.before = entry.before;
		decls.push_back(entry.decl);
	}

	auto built = graph::build(decls);
	if (not built.has_value()) {
		failGraph(built.error());
	}
	_graph = std::move(built).value();

	/* Bind every node to its slot and its pipeline stages */

	const auto count = _graph.nodes.size();
	_nodeStages.clear();
	_slotOfNode.clear();
	for (u64 idx = 0; idx < count; ++idx) {

		const auto& node = _graph.nodes[idx];
		const auto where = std::ranges::find(merged, string_view { node.name }, [](ref<const Merged> entry_) { return entry_.decl.name; });
		::hg::assertrt(where != merged.end());
		_nodeStages.push_back(where->stages);

		const auto slot = static_cast<u16>(_stages.find(node.name)->second->getSlot());
		_slotOfNode.push_back(slot);
		_slots[slot]->setNode(static_cast<u16>(idx));
		_slots[slot]->setMask(graph::toMask(node.lane));
	}

	_pending = std::make_unique<std::atomic<u16>[]>(count);
}

void CompositePipeline::start() {

	::hg::assertrt(not _graph.nodes.empty());
	::hg::assertrt(not _stopping.test());

	for (u64 idx = 0; idx < _graph.nodes.size(); ++idx) {
		const auto dispatcher = _slots[_slotOfNode[idx]]->getStaticDispatcher();
		for (auto* const stage : _nodeStages[idx]) {
			stage->staticDispatch(dispatcher);
		}
	}

	startFrame();
}

void CompositePipeline::setMaxInFlight(const u32 count_) noexcept {
	// shortcut: only 0 (unbounded) and 1 (serial) supported; bounded window needs readyQueue (design 6.3)
	::hg::assertrt(count_ <= 1uL);
	// workers read _maxInFlight unsynchronised, so it may only change while no frame runs (before start() or after stop())
	::hg::assertrt(_remaining.load(std::memory_order::relaxed) == 0u);
	_maxInFlight = count_;
}

ref<const graph::Graph> CompositePipeline::graph() const noexcept {
	return _graph;
}

bool CompositePipeline::isStopping() const noexcept {
	return _stopping.test();
}

void CompositePipeline::stop() {
	::hg::assertrt(not _stopping.test());

	_stopping.test_and_set();
	_stopped.wait();

	// every runner call has returned before the pipeline may be destroyed
	while (_activeCalls.load(std::memory_order::acquire) != 0uL) {
		if (fiber::self::current() != nullptr) {
			fiber::self::yield();
		} else {
			thread::self::yield();
		}
	}

	_stopped.reset();
}

void CompositePipeline::destroy() {

	for (const auto& pipeline : _pipelines) {
		pipeline->dismount(*static_cast<ptr<StageRegister>>(this));
	}

}

const non_owning_rptr<Schedule> CompositePipeline::getSchedule() const noexcept {
	return _schedule;
}

Opt<ref<const Stage>> CompositePipeline::registerStage(mref<uptr<PipelineStage>> stage_) {

	const auto& identifier = stage_->stageIdentifier();
	_pipelineStages.emplace_back(std::move(stage_));

	/**/

	const auto where = _stages.find(identifier.value);
	if (where != _stages.end()) {
		const auto& stored = where->second;
		stored->incRef();
		return Some(*stored);
	}

	/**/

	auto slot = make_uptr<CompositeSlot>(this, _schedule);

	const auto slotIter = _slots.insert(_slots.end(), std::move(slot));
	const auto slotBegin = _slots.begin();
	const auto slotIdx = std::distance(slotBegin, slotIter);

	/**/

	::hg::assertrt(0LL <= slotIdx && slotIdx <= 127LL);

	const auto result = _stages.insert_or_assign(
		identifier.value,
		make_uptr<Stage>(clone(identifier.value), static_cast<s8>(slotIdx))
	);

	return Some(*result.first->second);
}

Opt<ref<const Stage>> CompositePipeline::getStage(cref<string> identifier_) const noexcept {
	const auto where = _stages.find(identifier_);
	return where != _stages.end() ? Some(*where->second) : None;
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

void CompositePipeline::startFrame() {

	// no node is in flight here; pushing the first root task publishes these stores to the workers
	const auto count = _graph.nodes.size();
	for (u64 idx = 0; idx < count; ++idx) {
		_pending[idx].store(_graph.nodes[idx].predecessors, std::memory_order::relaxed);
	}
	_remaining.store(static_cast<u16>(count), std::memory_order::relaxed);

	if (_maxInFlight == 1uL) {
		dispatch(_graph.order.front());
		return;
	}

	for (u64 idx = 0; idx < count; ++idx) {
		if (_graph.nodes[idx].predecessors == 0u) {
			dispatch(static_cast<u16>(idx));
		}
	}
}

void CompositePipeline::dispatch(const u16 node_) {

	auto* const slot = _slots[_slotOfNode[node_]].get();
	const auto dispatcher = slot->getDynamicDispatcher();
	for (auto* const stage : _nodeStages[node_]) {
		stage->dynamicDispatch(dispatcher);
	}

	slot->submit();
}

void CompositePipeline::complete(const non_owning_rptr<CompositeSlot> slot_) {

	// acq_rel: a read-modify-write before this call's _remaining decrement, so a frame end that sets _stopped already sees it counted
	_activeCalls.fetch_add(1uL, std::memory_order::acq_rel);

	auto* const schedule = _schedule;
	const auto node = slot_->getNode();
	const bool serial = _maxInFlight == 1uL;

	for (const auto successor : _graph.nodes[node].successors) {
		const auto prev = _pending[successor].fetch_sub(1u, std::memory_order::acq_rel);
		if (prev == 1u && not serial) {
			dispatch(successor);
		}
	}

	const auto prevRemaining = _remaining.fetch_sub(1u, std::memory_order::acq_rel);

	if (prevRemaining == 1u) {
		if (_stopping.test()) {
			_stopping.clear();
			_stopped.set();
		} else {
			startFrame();
		}

	} else if (serial) {
		// serial mode runs the topological order; (count - remaining) nodes are done, so that index is next
		const auto done = _graph.order.size() - (prevRemaining - 1u);
		dispatch(_graph.order[done]);
	}

	// last access to this; stop() waits for zero before the pipeline may be destroyed
	_activeCalls.fetch_sub(1uL, std::memory_order::release);

	schedule->tick();
}

void CompositePipeline::pushTask(
	const non_owning_rptr<const Stage> stage_,
	mref<task_handle_type> task_
) {
	_slots[stage_->getSlot()]->dynamicEnqueue(std::move(task_));
}
