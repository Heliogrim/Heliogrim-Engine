#include "ReflowEngine.hpp"

#include <Engine.Common/Collection/Stack.hpp>
#include <Engine.GFX/Aabb.hpp>

#include "Widget/Widget.hpp"
#include "ReflowState.hpp"
#include "Children.hpp"

using namespace hg::engine::reflow;
using namespace hg;

std::atomic_uint_fast16_t ReflowEngine::_globalReflowTick = 0;

/**/

static void recomputeStates(ref<ReflowState> state_, cref<sptr<Widget>> root_);

static void reapplyLayout(ref<ReflowState> state_, cref<sptr<Widget>> root_, mref<LayoutContext> globalCtx_);

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
    recomputeStates(state_, widget_);

    /**
     * Second Pass: Apply layout changes
     */
    reapplyLayout(state_, widget_, std::move(globalCtx_));

}

/**/

void recomputeStates(ref<ReflowState> state_, cref<sptr<Widget>> root_) {

    constexpr float scale = 1.F;

    /**
     * Use depth-first visiting to guarantee bottom-up computed sizes
     */
    Stack<sptr<Widget>> pending {};
    pending.push(root_);

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
            pending.push(child);

            /* Warning: Temporary solution */

            if (child->shouldTick()) {
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

        auto* const widgetState = state_.record(cur);
        const auto prefetchedSize = cur->prefetchDesiredSize(state_, scale);

        /**
         * Preserve previous layout state as last aabb
         */
        widgetState->lastAabb.min = widgetState->layoutOffset;
        widgetState->lastAabb.max = widgetState->layoutOffset + widgetState->layoutSize;

        /**
         * Compare cached/preserved and computed for state changes
         */
        if (prefetchedSize != widgetState->prefetchedSize) {
            // TODO: Replace with correct function
            //cur->markAsPending();
        }

        widgetState->prefetchedSize = prefetchedSize;

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

void reapplyLayout(ref<ReflowState> state_, cref<sptr<Widget>> root_, mref<LayoutContext> globalCtx_) {

    constexpr float scale = 1.F;

    /**
     * Use board-first visiting/computation
     */
    Stack<sptr<Widget>> pending {};
    pending.push(root_);

    Vector<engine::gfx::Aabb2d> revealed {};

    /**/

    while (not pending.empty()) {

        const auto cur = pending.top();
        pending.pop();

        /**
         * Apply layout computation
         */

        LayoutContext ctx {};
        if (not cur->hasParent()) {
            ctx = globalCtx_;

            auto* const rootState = state_.getStateOf(cur);
            rootState->referenceSize = ctx.localSize;
            rootState->cachedPreservedSize = rootState->prefetchedSize;
            rootState->layoutSize = rootState->prefetchedSize;
            rootState->layoutOffset = ctx.localOffset;

        } else {
            const auto* const widgetState = state_.getStateOf(cur);
            ctx = LayoutContext {
                .localOffset = widgetState->layoutOffset,
                .localSize = widgetState->layoutSize,
                .localScale = scale
            };
        }

        /**
         * Recompute desired size of children ( used for relative calculations )
         */

        const auto* const children = cur->children();
        for (const auto& child : *children) {

            auto* const childState = state_.getStateOf(child);

            /**/

            // TODO: Calculate Reference Sizes for child states
            // TODO: We might need a method `computeReference(math::vec2 reference_) -> math::vec2 (child-reference)`
            childState->referenceSize = ctx.localSize;
            // TODO: Calculate size limit for child states
            childState->layoutSize = ctx.localSize;

            /**/

            const auto computedSize = child->computeDesiredSize(*childState);
            childState->cachedPreservedSize = computedSize;
        }

        /**/

        cur->applyLayout(state_, std::move(ctx));

        /**
         *
         */

        // Warning: Temporary AABB Checks

        bool childAabbChanged = false;
        for (auto iter = cur->children()->rbegin(); iter != cur->children()->rend(); ++iter) {

            auto* const childState = state_.getStateOf(*iter);

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

        const auto pendingResult = cur->clearPending();
        if (pendingResult & WidgetStateFlagBits::ePending) {

            // TODO: Warning: If AABB of element changed, we need to queue previously covered elements to overdraw

            cur->updateRenderVersion(state_.getRenderTick());
        }

        const auto nextAabb = engine::gfx::Aabb2d {
            cur->layoutState().layoutOffset, cur->layoutState().layoutOffset + cur->layoutState().layoutSize
        };
        const auto changedAabb = nextAabb != cur->layoutState().lastAabb;
        if (changedAabb) {
            cur->updateRenderVersion(state_.getRenderTick());
        }

        // TODO: Check for layout changes -> drop sub-sequent layout operations

        if (
            not(pendingResult & WidgetStateFlagBits::ePending) &&
            not(pendingResult & WidgetStateFlagBits::ePendingInherit) &&
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

        for (const auto& child : *children) {
            pending.push(child);
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
