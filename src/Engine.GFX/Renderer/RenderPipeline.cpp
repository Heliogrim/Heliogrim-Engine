#include "RenderPipeline.hpp"

#include <Engine.Common/Collection/Set.hpp>

#include "HORenderPass.hpp"
#include "RenderStage.hpp"
#include "RenderStagePass.hpp"
#include "RenderStageProcessor.hpp"
#include "../Device/Device.hpp"

using namespace ember::engine::gfx::render;
using namespace ember::engine::gfx;
using namespace ember;

RenderPipeline::RenderPipeline() = default;

RenderPipeline::~RenderPipeline() = default;

void RenderPipeline::destroy() {
    /**
     * Iterate backward/reverse over all stages
     */
    for (auto it { _stages.rbegin() }; it != _stages.rend(); ++it) {
        it->get()->pipelineDestroy(this);
    }
}

void RenderPipeline::setup(cref<sptr<Device>> device_) {
    /**
     * Store back associated device
     */
    _device = device_;

    /**
     * Iterate forward over all stages
     */
    for (const auto& entry : _stages) {
        entry->pipelineSetup(this);
    }
}

void RenderPipeline::allocate(const ptr<HORenderPass> renderPass_) {
    /**
     * Iterate forward over all stages
     */
    for (const auto& entry : _stages) {
        entry->allocate(renderPass_);
    }
}

void RenderPipeline::free(const ptr<HORenderPass> renderPass_) {
    /**
     * Iterate backward/reverse over all stages
     */
    for (auto it { _stages.rbegin() }; it != _stages.rend(); ++it) {
        it->get()->free(renderPass_);
    }
}

void RenderPipeline::invoke(const non_owning_rptr<HORenderPass> renderPass_) const {

    Vector<CommandBatch> contextBatches { _STD move(renderPass_->batches()) };

    /**
     * Track dispatch resources
     */
    Vector<ptr<RenderStagePass>> stagePasses {};
    Vector<ptr<RenderStageProcessor>> stageProcessors {};

    stagePasses.reserve(_stages.size());
    stageProcessors.reserve(_stages.size());

    /**
     * Iterate forward over all sorted stages
     *  TODO: check for dependencies so we can execute multiple stages at the same time
     */
    for (const auto& stage : _stages) {

        /**
         * Prepare context data and worker
         */
        const auto stagePass {
            make_ptr<RenderStagePass>(stage.get())
        };
        const auto stageProcessor {
            make_ptr<RenderStageProcessor>(&stage->multiplexer())
        };

        stagePasses.push_back(stagePass);
        stageProcessors.push_back(stageProcessor);

        /**
         * Configure stage processor
         *  TODO: We might cache processor with it's configuration
         */
        stageProcessor->use(renderPass_);
        stageProcessor->use(stagePass);

        /**
         * Dispatch the invocation
         */
        // Warning: When using address, ensure each cycle gets it's own StagePass instance with a unique address.
        // Warning: Might use a caching mechanism to prevent multiple heap allocations per HORenderPass
        stageProcessor->dispatch();
        // TODO: await(stageProcessor.dispatch(renderPass_, stagePass));
    }

    /**
     * Await processor dispatches
     */
    for (auto& entry : stageProcessors) {
        delete entry;
        entry = nullptr;
    }

    stageProcessors.clear();

    /**
     * Collect and Post-Process Commands
     *
     *  -> Check for propagation of dependencies between Render Stages to api synchronization.
     *      -> If some stages should be synchonized, create semaphores to enforce device local ordering.
     *  -> If any of the previous command batches of the HORenderPass contains signals and/or barriers,
     *      interleave them into context batches
     */
    for (auto& entry : stagePasses) {

        /**
         * Collect and Merge CommandBatches
         */
        if (!entry->batch().empty()) {

            // TODO: Check whether we need support for multiple CommandBatches per RenderStage
            // TODO: Merge CommandBatches of StagePass to HORenderPass
            contextBatches.push_back(entry->batch());
        }

        // Release
        delete entry;
        entry = nullptr;
    }

    stagePasses.clear();

    // TODO:
    renderPass_->batches() = _STD move(contextBatches);
}

inline bool guaranteeUnique(cref<Vector<sptr<RenderStage>>> stages_, cref<sptr<RenderStage>> stage_) {
    const auto it { _STD find(stages_.begin(), stages_.end(), stage_) };
    return it == stages_.end();
}

cref<Vector<sptr<RenderStage>>> RenderPipeline::stages() const noexcept {
    return _stages;
}

RenderPipelineValidationResult RenderPipeline::push(cref<sptr<RenderStage>> stage_) {

    if (!guaranteeUnique(_stages, stage_)) {
        return RenderPipelineValidationResult::eFailedUnique;
    }

    if (_stages.empty()) {
        _stages.push_back(stage_);
        return RenderPipelineValidationResult::eSuccess;
    }

    CompactSet<ptr<RenderStage>> predecessors {};
    CompactSet<ptr<RenderStage>> successors {};

    /**
     * Resolve dependency of new stage
     */
    for (const auto& dependency : stage_->dependencies()) {
        const auto exists {
            _STD find_if(
                _stages.begin(),
                _stages.end(),
                [&stage = dependency.stage](cref<sptr<RenderStage>> entry_) {
                    return entry_.get() == stage;
                }
            ) != _stages.end()
        };

        /**
         * Don't prevent pushing with unsatified require dependecies
         */
        if (!exists) {
            continue;
        }

        if (dependency.order == RenderStageOrder::ePredecessor) {
            predecessors.insert(dependency.stage);
        } else if (dependency.order == RenderStageOrder::eSuccessor) {
            successors.insert(dependency.stage);
        }
    }

    /**
     * Resolve dependency of old stages
     */
    for (const auto& stored : _stages) {

        const auto& dependencies { stored->dependencies() };
        auto depIt {
            _STD find_if(
                dependencies.begin(),
                dependencies.end(),
                [stage = stage_.get()](cref<RenderStageDependency> entry_) {
                    return entry_.stage == stage;
                }
            )
        };

        if (depIt == dependencies.end()) {
            continue;
        }

        if (depIt->order == RenderStageOrder::ePredecessor) {
            successors.insert(stored.get());
        } else if (depIt->order == RenderStageOrder::eSuccessor) {
            predecessors.insert(stored.get());
        }
    }

    /**
     * Check for intersection of successors and predecessor ~ dependency collision
     */
    for (const auto& successor : successors) {
        if (predecessors.contains(successor)) {
            return RenderPipelineValidationResult::eFailedOrder;
        }
    }

    /**
     * Use predecessor to find suitable position to store new stage
     */
    auto pos { _stages.begin() };
    for (auto it { _stages.begin() }; it != _stages.end(); ++it) {
        if (predecessors.contains((*it).get())) {
            pos = it;
        }
    }

    /**
     * Store new stage
     */
    if (++pos != _stages.end()) {
        _stages.insert(++pos, stage_);
    } else {
        _stages.push_back(stage_);
    }

    return RenderPipelineValidationResult::eSuccess;
}

bool RenderPipeline::pop(cref<sptr<RenderStage>> stage_) {

    const auto it { _STD remove(_stages.begin(), _stages.end(), stage_) };

    if (it == _stages.end()) {
        return false;
    }

    _stages.erase(it, _stages.end());
    return true;
}

RenderPipelineValidationResult RenderPipeline::validate() const noexcept {
    /**
     * Order is guaranteed by push function, so we don't need to check again
     */

    /**
     * Collect all required stage dependencies
     */
    CompactSet<ptr<RenderStage>> required {};
    for (const auto& stage : _stages) {
        for (const auto& dep : stage->dependencies()) {
            if (dep.required) {
                required.insert(dep.stage);
            }
        }
    }

    /**
     * Validate every stage is present
     */
    for (const auto& stage : _stages) {
        required.erase(stage.get());
    }

    return required.empty() ?
               RenderPipelineValidationResult::eSuccess :
               RenderPipelineValidationResult::eFailedRequired;
}

cref<sptr<Device>> RenderPipeline::device() const noexcept {
    return _device;
}
