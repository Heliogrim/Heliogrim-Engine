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
        if (stage.getExecState().done() || stage.getExecState().running()) {
            ++cursor_;
            continue;
        }

        const auto cnd = stage.ready();
        if (cnd) {

            if (stage.start()) {
                return &stage;
            }

            return {};
        }

        assert(
            stage.getExecState().state.load(_STD memory_order::consume) ==
            static_cast<_STD underlying_type_t<ExecStateFlag>>(ExecStateFlag::eUndefined)
        );
        assert(not cnd);
        return {};
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

            // If selected is not null, it was implicitly started via CAS expression

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

void RenderPipeline::regSubState(nmpt<State> state_) const {

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
