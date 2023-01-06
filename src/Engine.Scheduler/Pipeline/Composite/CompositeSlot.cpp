#include "CompositeSlot.hpp"

#include <ranges>

#include "CompositeStage.hpp"
#include "../CompositePipeline.hpp"
#include "../../Process/Schedule.hpp"

using namespace ember::engine::scheduler;
using namespace ember;

CompositeSlot::CompositeSlot(
    const non_owning_rptr<CompositePipeline> pipeline_,
    const non_owning_rptr<Schedule> schedule_,
    const non_owning_rptr<CompositeStage> stage_
) :
    _schedule(schedule_),
    _stage(stage_),
    _pipeline(pipeline_),
    _mtx() {}

CompositeSlot::~CompositeSlot() = default;

const non_owning_rptr<Schedule> CompositeSlot::getSchedule() const noexcept {
    return _schedule;
}

const non_owning_rptr<CompositeStage> CompositeSlot::getCompositeStage() const noexcept {
    return _stage;
}

void CompositeSlot::setCompositeStage(const non_owning_rptr<CompositeStage> stage_) {
    _stage = stage_;
}

void CompositeSlot::staticEnqueue(mref<non_owning_rptr<const task::TaskDelegate>> task_) {

    assert(task_->ctrl() == nullptr);
    const_cast<ptr<task::TaskDelegate>>(task_)->ctrl() = static_cast<ptr<TaskCtrl>>(this);

    concurrent::SpinLock lck { _mtx };
    _staticTasks.push_back(_STD move(task_));
}

void CompositeSlot::dynamicEnqueue(mref<non_owning_rptr<const task::TaskDelegate>> task_) {

    assert(task_->ctrl() == nullptr);
    const_cast<ptr<task::TaskDelegate>>(task_)->ctrl() = static_cast<ptr<TaskCtrl>>(this);

    concurrent::SpinLock lck { _mtx };
    _dynamicTasks.push_back(_STD move(task_));
}

StageDispatcher CompositeSlot::getStaticDispatcher() const noexcept {
    return StageDispatcher {
        const_cast<ptr<CompositeSlot>>(this),
        &CompositeSlot::staticEnqueue
    };
}

StageDispatcher CompositeSlot::getDynamicDispatcher() const noexcept {
    return StageDispatcher {
        const_cast<ptr<CompositeSlot>>(this),
        &CompositeSlot::dynamicEnqueue
    };
}

static void disposeTask(const non_owning_rptr<const task::TaskDelegate> task_) {

    switch (task_->type()) {
        case task::TaskType::eTickTask:
        case task::TaskType::eTask: {
            delete static_cast<ptr<const task::Task>>(task_);
            break;
        }
        case task::TaskType::eRepetitive: {
            delete static_cast<ptr<const task::RepetitiveTask>>(task_);
            break;
        }
        case task::TaskType::eBatch: {
            delete static_cast<ptr<const task::BatchTask>>(task_);
            break;
        }
            [[unlikely]] default: { }
    }

}

void CompositeSlot::onComplete(const non_owning_rptr<const task::TaskDelegate> task_) {

    const auto prev = _signals.fetch_sub(1, _STD memory_order_release);

    /**/

    const bool isStaticTask = _STD ranges::contains(_staticTasks, task_);
    if (not isStaticTask) {
        // TODO: Dispose
        disposeTask(task_);
    }

    /**/

    if (prev != 1ui16) {
        return;
    }

    _pipeline->complete(this);
}

void CompositeSlot::submit() {

    concurrent::SpinLock lck { _mtx };

    /**/

    for (const auto* const task : _staticTasks) {

        if (task->type() == task::TaskType::eBatch) {
            // TODO: _signals.fetch_add( << batch size >>, _STD memory_order_relaxed);
            _signals.fetch_add(1ui16, _STD memory_order_relaxed);

        } else {

            _signals.fetch_add(1ui16, _STD memory_order_relaxed);
        }
    }

    for (const auto* const task : _dynamicTasks) {

        if (task->type() == task::TaskType::eBatch) {
            // TODO: _signals.fetch_add( << batch size >>, _STD memory_order_relaxed);
            _signals.fetch_add(1ui16, _STD memory_order_relaxed);

        } else {

            _signals.fetch_add(1ui16, _STD memory_order_relaxed);
        }
    }

    /* TODO: Rewrite quickfix */

    if (_signals.load(_STD memory_order_relaxed) == 0ui16) {
        // Attention: Currently using a dummy to guarantee progress
        _signals.fetch_add(1ui16, _STD memory_order_relaxed);

        auto dummy = task::make_task([] {});
        const_cast<ptr<task::TaskDelegate>>(dummy)->ctrl() = static_cast<ptr<TaskCtrl>>(this);
        _schedule->push(_STD move(dummy));
        return;
    }

    /**/

    for (const auto* const task : _staticTasks) {
        _schedule->push(task);
    }

    for (auto&& task : _dynamicTasks) {
        _schedule->push(_STD move(task));
    }
    _dynamicTasks.clear();
}
