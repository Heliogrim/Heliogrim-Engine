#include "ReflowEngine.hpp"

#include <Engine.Common/Collection/Stack.hpp>
#include <Engine.GFX/Aabb.hpp>

#include "Children.hpp"
#include "ReflowState.hpp"
#include "Widget/Widget.hpp"

using namespace hg::engine::reflow;
using namespace hg;

std::atomic_uint_fast16_t ReflowEngine::_globalReflowTick = 0;

/**/

static void prefetchStateAlongAxis(
	ReflowAxis axis_,
	ref<const SharedPtr<Widget>> root_,
	ref<ReflowState> state_
);

static void computeStateAlongAxis(
	ReflowAxis axis_,
	ref<const LayoutContext> globalCtx_,
	ref<const SharedPtr<Widget>> root_,
	ref<ReflowState> state_
);

static void finalizeLayoutAndStates(
	ref<const SharedPtr<Widget>> root_,
	ref<ReflowState> state_,
	mref<LayoutContext> globalCtx_
);

/**/

void ReflowEngine::updateTickVersion() noexcept {
	++ReflowEngine::_globalReflowTick;
}

u16 ReflowEngine::getGlobalRenderTick() noexcept {
	return ReflowEngine::_globalReflowTick.load(std::memory_order::relaxed);
}

void ReflowEngine::tick(ref<ReflowState> state_, cref<sptr<Widget>> widget_, mref<LayoutContext> globalCtx_) {

	// TODO: Erase temporary cleanup
	state_.reset();

	/**
	 * First Pass: Recording prefetched desired size and distribute style updates
	 */
	state_.reset();
	prefetchStateAlongAxis(ReflowAxis::eXAxis, widget_, state_);
	computeStateAlongAxis(ReflowAxis::eXAxis, globalCtx_, widget_, state_);

	state_.reset();
	prefetchStateAlongAxis(ReflowAxis::eYAxis, widget_, state_);
	computeStateAlongAxis(ReflowAxis::eYAxis, globalCtx_, widget_, state_);

	/**
	 * Second Pass: Apply layout changes
	 */
	finalizeLayoutAndStates(widget_, state_, std::move(globalCtx_));
}

/**/

void prefetchStateAlongAxis(
	ReflowAxis axis_,
	ref<const SharedPtr<Widget>> root_,
	ref<ReflowState> state_
) {

	/**
	 * Use depth-first visiting to guarantee bottom-up computed sizes
	 */
	auto cont = Vector<nmpt<Widget>>();
	cont.reserve(16uLL);
	auto pending = Stack<nmpt<Widget>, Vector<nmpt<Widget>>> { ::hg::move(cont) };
	pending.emplace(root_.get());

	while (not pending.empty()) {

		const auto cur = pending.top();
		const auto* const children = cur->children();

		/**
		 * Check for dependencies -> depth-first
		 */
		bool dependencies = false;
		for (const auto& child : *children) {

			if (state_.getStateOf(child) != nullptr) {
				continue;
			}

			dependencies = true;
			pending.emplace(child.get());

			/* Warning: Temporary solution */

			if (child->shouldTick() && axis_ == ReflowAxis::eXAxis) {
				child->updateRenderVersion(state_.getRenderTick());
				child->tick();
			}
		}

		/**
		 * Check whether every child was computed
		 */

		if (dependencies) {
			continue;
		}

		/**
		 * Record and compute current widget
		 */
		const auto widgetState = state_.record(cur);
		const auto sizing = cur->prefetchSizing(axis_, state_);

		/**
		 * Preserve previous layout state as last aabb
		 */
		widgetState->lastAabb.min = widgetState->layoutOffset;
		widgetState->lastAabb.max = widgetState->layoutOffset + widgetState->layoutSize;

		/**
		 * Compare cached/preserved and computed for state changes
		 */
		if (sizing.sizing != widgetState->prefetchSize) {
			// TODO: Replace with correct function
			//cur->markAsPending();
		}

		/**
		 * Update widget's layout state with prefetched sizes
		 */
		if (axis_ == ReflowAxis::eXAxis) {
			widgetState->prefetchMinSize.x = sizing.minSizing.x;
			widgetState->prefetchSize.x = sizing.sizing.x;
		} else {
			widgetState->prefetchMinSize.y = sizing.minSizing.y;
			widgetState->prefetchSize.y = sizing.sizing.y;
		}

		/* Warning: Temporary Fix */
		if (state_.getRenderTick() == 0u) {
			cur->state().setRenderVersion(0uL);
			cur->state().setProxyRenderVersion(0uL);
		}

		/**
		 * Remove current element from work stack
		 */
		pending.pop();
	}
}

void computeStateAlongAxis(
	ReflowAxis axis_,
	ref<const LayoutContext> globalCtx_,
	ref<const SharedPtr<Widget>> root_,
	ref<ReflowState> state_
) {

	/**
	 * Use broad-first visiting/computation
	 */
	auto cont = Vector<nmpt<Widget>>();
	cont.reserve(32uLL);
	auto pending = Stack<nmpt<Widget>, Vector<nmpt<Widget>>> { ::hg::move(cont) };
	pending.emplace(root_.get());

	root_->getLayoutState().computeSize = globalCtx_.localSize;

	while (not pending.empty()) {

		const auto cur = pending.top();
		pending.pop();

		/**
		 * Forward update constraints and compute sizing state
		 */
		const auto referenceSize = cur->hasParent() ? cur->computeReferenceSize(axis_) : globalCtx_.localSize;

		/**/

		const auto* const children = cur->children();
		for (const auto& child : *children) {

			const auto childState = state_.getStateOf(child);

			/**/

			if (axis_ == ReflowAxis::eXAxis) {
				childState->referenceSize.x = referenceSize.x;
			} else {
				childState->referenceSize.y = referenceSize.y;
			}
			const auto sizing = child->passPrefetchSizing(axis_, *childState);

			/**
			 * Update widget's layout state with prefetched sizes
			 */
			if (axis_ == ReflowAxis::eXAxis) {
				childState->prefetchMinSize.x = sizing.minSizing.x;
				childState->prefetchSize.x = sizing.sizing.x;
				childState->prefetchMaxSize.x = sizing.maxSizing.x;
			} else {
				childState->prefetchMinSize.y = sizing.minSizing.y;
				childState->prefetchSize.y = sizing.sizing.y;
				childState->prefetchMaxSize.y = sizing.maxSizing.y;
			}

			/**
			 * Queue compute sized children for computation/recursion
			 *	Note: This is hoisted from the end of this compute function, as the computeSizing is not allowed to
			 *			manipulate the specific set of children while scheduling
			 */
			pending.emplace(child.get());
		}

		/**/

		cur->computeSizing(axis_, *state_.getStateOf(cur));
	}
}

void finalizeLayoutAndStates(ref<const SharedPtr<Widget>> root_, ref<ReflowState> state_, mref<LayoutContext> globalCtx_) {

	/**
	 * Use broad-first visiting/computation
	 */
	auto cont = Vector<nmpt<Widget>>();
	cont.reserve(32uLL);
	auto pending = Stack<nmpt<Widget>, Vector<nmpt<Widget>>> { ::hg::move(cont) };
	pending.emplace(root_.get());

	Vector<engine::gfx::Aabb2d> revealed {};

	/**/

	while (not pending.empty()) {

		const auto cur = pending.top();
		pending.pop();

		/**
		 * Apply layout computation
		 */

		if (not cur->hasParent()) {
			const auto rootState = state_.getStateOf(cur);
			rootState->layoutSize = rootState->computeSize;
			rootState->layoutOffset = globalCtx_.localOffset;
		}

		/**
		 * Apply layout computation and position elements
		 */
		cur->applyLayout(state_);

		/**
		 *
		 */

		// Warning: Temporary AABB Checks

		bool childAabbChanged = false;
		for (auto iter = cur->children()->rbegin(); iter != cur->children()->rend(); ++iter) {

			const auto childState = state_.getStateOf(*iter);

			const auto prevAabb = childState->lastAabb;
			auto nextAabb = engine::gfx::Aabb2d {
				childState->layoutOffset, childState->layoutOffset + childState->layoutSize
			};

			// TODO: AABB has to be limited by parent scissor element
			const auto clipAabb = state_.getStateOf(cur)->lastAabb;
			//nextAabb.min = math::compMax(nextAabb.min, clipAabb.min);
			//nextAabb.max = math::compMin(nextAabb.max, clipAabb.max);

			if (nextAabb == prevAabb || *iter == NullWidget::instance()) {
				continue;
			}

			// TODO:
			//const auto diffAabb = prevAabb - nextAabb;
			const auto diffAabb = prevAabb;
			revealed.emplace_back(diffAabb);
			childAabbChanged = true;
		}

		/**/

		const auto nextAabb = engine::gfx::Aabb2d {
			cur->getLayoutState().layoutOffset, cur->getLayoutState().layoutOffset + cur->getLayoutState().layoutSize
		};

		const auto layoutPending = cur->clearLayoutPending();
		const auto renderPending = cur->clearRenderPending();
		const auto changedAabb = nextAabb != cur->getLayoutState().lastAabb;

		if (layoutPending || renderPending || changedAabb) {
			// TODO: Warning: If AABB of element changed, we need to queue previously covered elements to overdraw
			cur->updateRenderVersion(state_.getRenderTick());
		}

		// TODO: Check for layout changes -> drop sub-sequent layout operations
		if (
			not(layoutPending) &&
			not(renderPending) &&
			not(changedAabb) &&
			not(childAabbChanged)
		) {
			continue;
		}

		/**
		 * Queue layouted children for computation/recursion
		 */

		// TODO: Warning: We may want to combine the Aabb check and pending pushback into one single loop
		// -> This may reduce the number of touched elements even more, cause right now we drop the cascade
		//      after we entered an unchanged element
		for (const auto& child : *cur->children()) {
			pending.emplace(child.get());
		}
	}

	/**/

	if (revealed.empty()) {
		return;
	}

	size_t merged = 0uLL;
	for (auto iter = revealed.rbegin(); iter != revealed.rend(); ++iter) {
		for (auto checkIter = iter + 1; checkIter != revealed.rend(); ++checkIter) {

			if (checkIter->contains(*iter)) {
				++merged;
				break;
			}

			if (iter->contains(*checkIter)) {
				std::swap(*iter, *checkIter);
				++merged;
				break;
			}

		}
	}

	revealed.erase(revealed.end() - merged, revealed.end());

	/**/

	for (const auto& aabb : revealed) {
		root_->cascadeRenderVersion(state_.getRenderTick(), aabb);
	}
}
