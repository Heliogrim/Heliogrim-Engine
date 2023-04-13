#include "ReflowEngine.hpp"

#include <Engine.Common/Collection/Stack.hpp>

#include "Widget/Widget.hpp"
#include "ReflowState.hpp"
#include "Children.hpp"

using namespace hg::engine::reflow;
using namespace hg;

/**/

static void recomputeStates(ref<ReflowState> state_, cref<sptr<Widget>> root_);

static void reapplyLayout(ref<ReflowState> state_, cref<sptr<Widget>> root_, mref<LayoutContext> globalCtx_);

/**/

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
    reapplyLayout(state_, widget_, _STD move(globalCtx_));

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
        }

        /**
         * Check whether every child was computed
         */

        if (dependencies) {
            continue;
        }

        /**
         * Record and compute currrent widget
         */

        auto* const widgetState = state_.record(cur);
        const auto prefetchedSize = cur->prefetchDesiredSize(state_, scale);

        /**
         * Compare cached/preserved and computed for state changes
         */
        if (prefetchedSize != widgetState->prefetchedSize) {
            // TODO: Replace with correct function
            //cur->markAsPending();
        }

        widgetState->prefetchedSize = prefetchedSize;

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

    while (not pending.empty()) {

        const auto cur = pending.top();
        pending.pop();

        /**
         * Apply layout computation
         */

        LayoutContext ctx {};
        if (not cur->hasParent()) {
            ctx = globalCtx_;

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

        cur->applyLayout(state_, _STD move(ctx));

        /**
         *
         */

        // TODO: Check for layout changes -> drop sub-sequent layout operations

        /**
         * Queue layouted children for computation/recursion
         */

        for (const auto& child : *children) {
            pending.push(child);
        }
    }

}
