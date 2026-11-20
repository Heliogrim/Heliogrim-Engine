#include "ReflowEngine.hpp"

#include <Engine.Graphics/Aabb.hpp>

#include "Children.hpp"
#include "ReflowState.hpp"
#include "Widget/Widget.hpp"

import Engine.Common;

using namespace hg::engine::reflow;
using namespace hg;

std::atomic_uint_fast16_t ReflowEngine::_globalReflowTick = 0;

/**/

namespace {
	template <typename Vec_>
	auto& axisOf(ref<Vec_> vec_, const ReflowAxis axis_) noexcept;

	void resetToRaw(ref<ReflowPassState> state_, const ReflowAxis axis_) noexcept;

	bool hasLayoutOrSubtreeBit(ref<const Widget> widget_) noexcept;

	void resetRenderVersions(ref<const SharedPtr<Widget>> root_);

	void tickWidgets(ref<const SharedPtr<Widget>> root_, const u16 renderTick_);

	void prefetchStateAlongAxis(
		ReflowAxis axis_,
		ref<const SharedPtr<Widget>> root_,
		ref<ReflowState> state_
	);

	void computeStateAlongAxis(
		ReflowAxis axis_,
		ref<const LayoutContext> globalCtx_,
		ref<const SharedPtr<Widget>> root_,
		ref<ReflowState> state_
	);

	void finalizeLayoutAndStates(
		ref<const SharedPtr<Widget>> root_,
		ref<ReflowState> state_,
		mref<LayoutContext> globalCtx_
	);
}

/**/

void ReflowEngine::updateTickVersion() noexcept {
	++ReflowEngine::_globalReflowTick;
}

u16 ReflowEngine::getGlobalRenderTick() noexcept {
	return ReflowEngine::_globalReflowTick.load(std::memory_order::relaxed);
}

void ReflowEngine::revealArea(
	ref<Widget> root_,
	ref<const graphics::Aabb2d> aabb2d_
) {
	root_.cascadeRenderVersion(getGlobalRenderTick(), aabb2d_);
}

void ReflowEngine::tick(ref<ReflowState> state_, cref<sptr<Widget>> widget_, mref<LayoutContext> globalCtx_) {

	// A wrapped tick could equal stale versions in clean subtrees, so the whole tree restarts from 0 and renders once
	if (state_.getRenderTick() == 0u) {
		resetRenderVersions(widget_);
	}

	/**
	 * Tickers run every frame, even inside subtrees the layout skips
	 */
	tickWidgets(widget_, state_.getRenderTick());

	const auto& rootLayout = widget_->getLayoutState();
	if (rootLayout.computeSize != globalCtx_.localSize || rootLayout.layoutOffset != globalCtx_.localOffset) {
		widget_->markLayoutPending();
	}

	if (not widget_->state().isAnyPending()) {
		return;
	}

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

namespace {
	template <typename Vec_>
	auto& axisOf(ref<Vec_> vec_, const ReflowAxis axis_) noexcept {
		return axis_ == ReflowAxis::eXAxis ? vec_.x : vec_.y;
	}

	void resetToRaw(ref<ReflowPassState> state_, const ReflowAxis axis_) noexcept {
		axisOf(state_.prefetchMinSize, axis_) = axisOf(state_.rawPrefetchMinSize, axis_);
		axisOf(state_.prefetchSize, axis_) = axisOf(state_.rawPrefetchSize, axis_);
	}

	bool hasLayoutOrSubtreeBit(ref<const Widget> widget_) noexcept {
		return widget_.state().isLayoutPending() || widget_.state().isSubtreePending();
	}

	void resetRenderVersions(ref<const SharedPtr<Widget>> root_) {

		auto cont = Vector<nmpt<Widget>>();
		cont.reserve(16uLL);
		auto pending = Stack<nmpt<Widget>, Vector<nmpt<Widget>>> { ::hg::move(cont) };
		pending.emplace(root_.get());

		while (not pending.empty()) {

			const auto cur = pending.top();
			pending.pop();

			cur->state().setRenderVersion(0u);
			cur->state().setProxyRenderVersion(0u);

			for (const auto& child : *cur->children()) {
				if (child != NullWidget::instance()) {
					pending.emplace(child.get());
				}
			}
		}
	}

	void tickWidgets(ref<const SharedPtr<Widget>> root_, const u16 renderTick_) {

		auto cont = Vector<nmpt<Widget>>();
		cont.reserve(16uLL);
		auto pending = Stack<nmpt<Widget>, Vector<nmpt<Widget>>> { ::hg::move(cont) };
		pending.emplace(root_.get());

		while (not pending.empty()) {

			const auto cur = pending.top();
			pending.pop();

			for (const auto& child : *cur->children()) {
				if (child->shouldTick()) {
					child->updateRenderVersion(renderTick_);
					child->tick();
				}
				pending.emplace(child.get());
			}
		}
	}

	void prefetchStateAlongAxis(
		ReflowAxis axis_,
		ref<const SharedPtr<Widget>> root_,
		ref<ReflowState> state_
	) {

		/**
		 * Use depth-first visiting to guarantee bottom-up computed sizes; only descend through pending nodes
		 */
		auto cont = Vector<nmpt<Widget>>();
		cont.reserve(16uLL);
		auto pending = Stack<nmpt<Widget>, Vector<nmpt<Widget>>> { ::hg::move(cont) };
		pending.emplace(root_.get());

		while (not pending.empty()) {

			const auto cur = pending.top();
			const auto curLayoutPending = cur->state().isLayoutPending();

			bool dependencies = false;
			for (const auto& child : *cur->children()) {

				// A layout-pending parent reads the unresolved value of children it will not re-measure
				if (curLayoutPending && not child->state().isLayoutPending()) {
					resetToRaw(child->getLayoutState(), axis_);
				}

				if (state_.isRecorded(child.get())) {
					continue;
				}

				if (hasLayoutOrSubtreeBit(*child)) {
					dependencies = true;
					pending.emplace(child.get());
					continue;
				}

				std::ignore = state_.record(child.get());
			}

			if (dependencies) {
				continue;
			}

			/**
			 * Record and compute current widget
			 */
			const auto widgetState = state_.record(cur);

			if (curLayoutPending) {
				const auto sizing = cur->prefetchSizing(axis_, state_);
				state_.countLayoutCall();

				axisOf(widgetState->rawPrefetchMinSize, axis_) = axisOf(sizing.minSizing, axis_);
				axisOf(widgetState->rawPrefetchSize, axis_) = axisOf(sizing.sizing, axis_);
				// Under a subtree-only parent this is a boundary, whose absolute min == max makes passPrefetchSizing the identity
				resetToRaw(cur->getLayoutState(), axis_);
			}

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

			auto& curState = cur->getLayoutState();
			const auto run = cur->state().isLayoutPending() ||
				axisOf(curState.referenceSize, axis_) != axisOf(curState.cachedReferenceSize, axis_) ||
				axisOf(curState.computeSize, axis_) != axisOf(curState.cachedComputeSize, axis_);

			if (not run) {
				for (const auto& child : *cur->children()) {
					if (hasLayoutOrSubtreeBit(*child)) {
						pending.emplace(child.get());
					}
				}
				continue;
			}

			/**
			 * Forward update constraints and compute sizing state
			 */
			const auto referenceSize = cur->hasParent() ? cur->computeReferenceSize(axis_) : globalCtx_.localSize;

			for (const auto& child : *cur->children()) {

				auto& childState = child->getLayoutState();

				axisOf(childState.referenceSize, axis_) = axisOf(referenceSize, axis_);
				resetToRaw(childState, axis_);
				const auto sizing = child->passPrefetchSizing(axis_, childState);

				axisOf(childState.prefetchMinSize, axis_) = axisOf(sizing.minSizing, axis_);
				axisOf(childState.prefetchSize, axis_) = axisOf(sizing.sizing, axis_);
				axisOf(childState.prefetchMaxSize, axis_) = axisOf(sizing.maxSizing, axis_);

				/**
				 * Queue compute sized children for computation/recursion
				 *	Note: This is hoisted from the end of this compute function, as the computeSizing is not allowed to
				 *			manipulate the specific set of children while scheduling
				 */
				pending.emplace(child.get());
			}

			cur->computeSizing(axis_, curState);
			state_.countLayoutCall();

			axisOf(curState.cachedReferenceSize, axis_) = axisOf(curState.referenceSize, axis_);
			axisOf(curState.cachedComputeSize, axis_) = axisOf(curState.computeSize, axis_);

			// Finalize applies it, and after X the Y prefetch re-measures it against the new width
			cur->state().set(WidgetStateFlagBits::eLayoutPending);
		}
	}

	void finalizeLayoutAndStates(ref<const SharedPtr<Widget>> root_, ref<ReflowState> state_, mref<LayoutContext> globalCtx_) {

		Vector<engine::graphics::Aabb2d> revealed {};

		/**
		 * Root is placed by the context; track its aabb like a child's
		 */
		{
			auto& rootState = root_->getLayoutState();
			rootState.layoutSize = rootState.computeSize;
			rootState.layoutOffset = globalCtx_.localOffset;

			const auto nextAabb = engine::graphics::Aabb2d {
				.min = rootState.layoutOffset, .max = rootState.layoutOffset + rootState.layoutSize
			};
			if (nextAabb != rootState.lastAabb) {
				rootState.lastAabb = nextAabb;
				root_->state().set(WidgetStateFlagBits::eLayoutPending);
			}
		}

		/**
		 * Use broad-first visiting/computation
		 */
		auto cont = Vector<nmpt<Widget>>();
		cont.reserve(32uLL);
		auto pending = Stack<nmpt<Widget>, Vector<nmpt<Widget>>> { ::hg::move(cont) };
		pending.emplace(root_.get());

		while (not pending.empty()) {

			const auto cur = pending.top();
			pending.pop();

			// Clear before applying, so marks raised by applyLayout or its children survive to the next tick
			const auto layoutPending = cur->clearLayoutPending();
			const auto renderPending = cur->clearRenderPending();
			std::ignore = cur->clearSubtreePending();

			if (layoutPending) {
				cur->applyLayout(state_);
				state_.countLayoutCall();
			}

			for (auto iter = cur->children()->rbegin(); iter != cur->children()->rend(); ++iter) {

				const auto& child = *iter;

				// Null is shared between trees; its aabb belongs to none of them
				if (child != NullWidget::instance()) {

					auto& childState = child->getLayoutState();
					const auto nextAabb = engine::graphics::Aabb2d {
						.min = childState.layoutOffset, .max = childState.layoutOffset + childState.layoutSize
					};

					// TODO: AABB has to be limited by parent scissor element
					if (nextAabb != childState.lastAabb) {
						// TODO: Reveal only the difference of prev and next aabb
						revealed.emplace_back(childState.lastAabb);
						childState.lastAabb = nextAabb;
						// applyLayout writes absolute positions, so a moved child must place its own children again
						child->state().set(WidgetStateFlagBits::eLayoutPending);
					}
				}

				if (child->state().isAnyPending()) {
					pending.emplace(child.get());
				}
			}

			if (layoutPending || renderPending) {
				cur->updateRenderVersion(state_.getRenderTick());
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

		const auto renderTick = state_.getRenderTick();
		for (const auto& aabb : revealed) {
			root_->cascadeRenderVersion(renderTick, aabb);
		}
	}
}
