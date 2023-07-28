#pragma once
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Collection/Vector.hpp>
#include <Engine.Common/Concurrent/SharedMemoryReference.hpp>

#include "RenderPipelineResult.hpp"
#include "__fwd.hpp"
#include "State/StateAware.hpp"

namespace hg::engine::gfx::render {
    class RenderPipeline final :
        public pipeline::StateAware {
    public:
        using this_type = RenderPipeline;

    public:
        RenderPipeline() noexcept = default;

        ~RenderPipeline() override = default;

    private:
        Vector<smr<pipeline::Stage>> _stages;

    public:
        [[nodiscard]] cref<Vector<smr<pipeline::Stage>>> getStages() const noexcept;

        void setStages(mref<Vector<smr<pipeline::Stage>>> stages_);

    private:
        [[nodiscard]] nmpt<pipeline::Stage> selectNext(decltype(_stages)::iterator cursor_) const;

        [[nodiscard]] decltype(_stages)::iterator moveNext(decltype(_stages)::iterator cursor_) const;

    private:
        _STD atomic_uint_fast8_t _startCnd;
        nmpt<const _STD atomic_uint_fast8_t> _endCnd;

    public:
        void reset();

        /**
         * Invocation function for this pipeline consuming a state to propagate
         *
         * @param state_ The state to process
         *
         * @returns A result flag and the state used to execute the pipeline.
         */
        [[nodiscard]] _STD pair<pipeline::RenderPipelineResult, uptr<pipeline::State>> operator()(
            _In_ mref<uptr<pipeline::State>> state_
        );

    public:
        void regSubState(nmpt<pipeline::State> state_) const override;

        [[nodiscard]] bool isReallocSupported() const noexcept override;

        bool alloc(nmpt<pipeline::State> state_) override;

        bool realloc(nmpt<pipeline::State> state_) override;

        bool free(nmpt<pipeline::State> state_) override;
    };
}
