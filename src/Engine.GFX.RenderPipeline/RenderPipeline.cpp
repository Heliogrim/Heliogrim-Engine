#include "RenderPipeline.hpp"

#include <Engine.Scheduler/Fiber/Fiber.hpp>
#include <Engine.Scheduler/Async.hpp>

#include "State/State.hpp"
#include "Stage/Stage.hpp"

using namespace hg::engine::gfx::render::pipeline;
using namespace hg::engine::gfx::render;
using namespace hg;

nmpt<Stage> RenderPipeline::selectNext(decltype(_stages)::iterator cursor_) const {

    while (cursor_ != _stages.end()) {

        auto& stage = **cursor_;

        const auto cnd = static_cast<ExecStateFlag>(stage.getExecState().cnd->load(std::memory_order::consume));
        switch (cnd) {
            case ExecStateFlag::eDone: {
                if (stage.getExecState().running()) {
                    ++cursor_;
                    continue;
                }

                // If the condition is fulfilled, schedule for execution
                // TODO: Check whether we should drop the direct precondition, cause we already rely on a CAS operation
                if (stage.start()) {
                    return &stage;
                }

                return {};
            }
            case ExecStateFlag::eRunning: {

                // As long as we iterate forward and the current stage is already executed, we can check direct neighbor stage for it's condition and execution
                // This was we may create something like a rolling execution wave through the pipeline
                if (stage.getExecState().running()) {
                    ++cursor_;
                    continue;
                }

                // If we encounter a stage with a running condition, but the state by itself is still stalled, we need to wait for another stage to finish.
                // Any further iteration will probably be dependent on still unexecuted or unknown conditions

                // TODO: Check whether we want to used the atomic exec state of the condition as conditional signal for the current fiber to yield

                // TODO: yield();
                // TODO: Could be fallthrough case
                return {};
            }
            default: {
                // If the state of the condition is unknown, break the iteration, cause we suspect that every following stage it also undefined.
                return {};
            }
        }
    }

    return {};
}

decltype(RenderPipeline::_stages)::iterator RenderPipeline::moveNext(decltype(_stages)::iterator cursor_) const {

    while (cursor_ != _stages.end() && cursor_->get()->getExecState().done()) {
        ++cursor_;
    }

    return cursor_;
}

void RenderPipeline::reset() {

    if (_startCnd.load() != static_cast<u8>(ExecStateFlag::eDone)) {
        return;
    }

    if (not _stages.empty() && _endCnd->load() != static_cast<u8>(ExecStateFlag::eDone)) {
        return;
    }

    for (const auto& stage : _stages) {
        stage->reset();
    }

    _startCnd.store(static_cast<u8>(ExecStateFlag::eUndefined), _STD memory_order::release);
}

_STD pair<RenderPipelineResult, uptr<State>> RenderPipeline::operator()(
    mref<uptr<State>> state_
) {

    auto expectStart = static_cast<u8>(ExecStateFlag::eUndefined);
    const bool success = _startCnd.compare_exchange_strong(
        expectStart,
        static_cast<u8>(ExecStateFlag::eDone),
        _STD memory_order::acq_rel
    );

    if (not success) {
        return _STD make_pair<>(RenderPipelineResult::eFailed, _STD move(state_));
    }

    assert(not _stages.empty());

    /**/

    auto cursor = _stages.begin();
    while (cursor != _stages.end()) {

        auto next = selectNext(cursor);
        if (next != nullptr) {

            // TODO: move execution of stage into fiber
            // Warning: We capture a variable `state_` with temporary lifetime; We have to guarantee that \
            //  The execution of each dispatched fiber was completed before leaving the function scope!?!
            #if FALSE
                scheduler::exec(
                    [next, &state_]() {

                        // Removed: next->start();
                        (*next)(state_.get());
                        next->finish();

                    }
                );
            }
            #endif

            // If selected is not null, in was implicitly started via CAS expression

            // Removed: next->start();
            (*next)(state_.get());
            next->finish();

            cursor = moveNext(cursor);
            continue;
        }

        #if FALSE
        scheduler::fiber::self::yield();
        #endif
        cursor = moveNext(cursor);
    }

    /**/

    // TODO: scheduler::waitUntilAtomic(*_endCnd, ExecStateFlag::eDone);
    constexpr auto expect = static_cast<u8>(ExecStateFlag::eDone);
    while (_endCnd->load(std::memory_order::relaxed) != expect) {
        scheduler::fiber::self::yield();
    }

    return _STD make_pair<>(RenderPipelineResult::eSuccess, _STD move(state_));
}

cref<Vector<smr<Stage>>> RenderPipeline::getStages() const noexcept {
    return _stages;
}

void RenderPipeline::setStages(mref<Vector<smr<Stage>>> stages_) {

    _endCnd = nullptr;
    _stages = _STD move(stages_);

    if (_stages.empty()) {
        return;
    }

    /**/

    _stages.front()->setExecStateCnd(&_startCnd);
    _endCnd = &_stages.back()->getExecState().state;
}

void RenderPipeline::regSubState(nmpt<State> state_) {

    for (const auto& stage : _stages) {

        const auto sa = stage->getStateAware();
        if (sa == nullptr) {
            continue;
        }

        sa->regSubState(state_);
    }
}

bool RenderPipeline::isReallocSupported() const noexcept {

    for (const auto& stage : _stages) {

        const auto sa = stage->getStateAware();
        if (sa == nullptr) {
            continue;
        }

        if (not sa->isReallocSupported()) {
            return false;
        }
    }

    return true;
}

bool RenderPipeline::alloc(nmpt<State> state_) {

    bool success = true;

    auto iter = _stages.begin();
    const auto end = _stages.end();
    for (; iter != end; ++iter) {

        const auto& stage = *iter;
        const auto sa = stage->getStateAware();

        if (sa == nullptr) {
            continue;
        }

        if (not(success = sa->alloc(state_))) {
            break;
        }
    }

    if (success) {
        return true;
    }

    /* Cleanup incomplete state */

    auto riter = _stages.rend();
    const auto rend = _stages.rend();

    riter -= (_STD distance(_stages.begin(), iter));

    for (; riter != rend; ++riter) {

        const auto& stage = *iter;
        const auto sa = stage->getStateAware();

        if (sa == nullptr) {
            continue;
        }

        // Ignore result of freeing, cause we are already in recover routine
        sa->free(state_);
    }

    return false;
}

bool RenderPipeline::realloc(nmpt<State> state_) {

    for (const auto& stage : _stages) {

        const auto sa = stage->getStateAware();
        if (sa == nullptr) {
            continue;
        }

        if (not sa->realloc(state_)) {
            return false;
        }
    }

    return true;
}

bool RenderPipeline::free(nmpt<State> state_) {

    bool success = true;

    for (auto iter = _stages.rbegin(); iter != _stages.rend(); ++iter) {

        const auto sa = (*iter)->getStateAware();
        if (sa == nullptr) {
            continue;
        }

        success = success && sa->free(state_);
    }

    return success;
}
