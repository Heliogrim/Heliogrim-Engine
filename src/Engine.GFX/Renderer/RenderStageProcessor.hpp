#pragma once
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Scene/RenderGraph.hpp>
#include <Engine.Scheduler/Fiber/Awaitable.hpp>

#include "__fwd.hpp"

namespace hg::engine::gfx::render {
    class RenderStageProcessor {
    public:
        using this_type = RenderStageProcessor;

        using dispatch_result_type = const ptr<scheduler::fiber::await_signal_sub_type>;

    public:
        /**
         * Constructor
         *
         * @author Julius
         * @date 30.03.2022
         */
        RenderStageProcessor() = default;

        /**
         * Constructor
         *
         * @author Julius
         * @date 30.03.2022
         *
         * @param multiplexer_ The underlying Multiplexer instance to process data
         */
        RenderStageProcessor(const non_owning_rptr<const Multiplexer> multiplexer_);

        /**
         * Destructor
         *
         * @author Julius
         * @date 30.03.2022
         */
        ~RenderStageProcessor() = default;

    public:
        /**
         * Dispatches this with given HORenderPass and RenderStagePass
         *
         * @author Julius
         * @date 30.03.2022
         *
         * @details Will schedule the invocation as strong guarantee task to support
         *  asynchronous processing with multiple stages. Be aware that this call might return immediately.
         *
         *  If pseudo synchronous invocation is required use fiber await on returned signal to block current execution path.
         *
         *  When mutating duplicated data from HORenderPass and RenderStagePass, try to prioritize RenderStageData, which will
         *      be post-merged to HORenderPass, which will prevent false sharing and cache invalidation when using
         *      multiple execution paths.
         *
         * @returns A dispatch result which reflects whether the operation is completed.
         */
        [[nodiscard]] dispatch_result_type dispatch() const;

    private:
        [[nodiscard]] bool operator()(const ptr<::hg::engine::scene::RenderGraph::node_type> node_) const;

    private:
        non_owning_rptr<HORenderPass> _renderPass;
        non_owning_rptr<RenderStagePass> _stagePass;

    public:
        void use(const non_owning_rptr<HORenderPass> renderPass_) noexcept;

        void use(const non_owning_rptr<RenderStagePass> renderStagePass_) noexcept;

    private:
        ptr<const Multiplexer> _multiplexer;

    public:
        [[nodiscard]] const ptr<const Multiplexer> multiplexer() const noexcept;

    private:
        mutable scheduler::fiber::await_signal_type _signal;
    };
}
