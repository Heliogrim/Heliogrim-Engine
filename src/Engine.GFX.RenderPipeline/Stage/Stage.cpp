#include "Stage.hpp"

#include "../State/StateAware.hpp"

using namespace hg::engine::gfx::render::pipeline;
using namespace hg;

void Stage::setExecStateCnd(mref<decltype(ExecState::cnd)> execStateCnd_) {
    _execState.cnd = _STD move(execStateCnd_);
}

cref<ExecState> Stage::getExecState() const noexcept {
    return _execState;
}

const nmpt<const StateAware> Stage::getStateAware() const noexcept {
    return const_cast<ptr<this_type>>(this)->getStateAware();
}

const nmpt<StateAware> Stage::getStateAware() noexcept {
    return {};
}

bool Stage::ready() const noexcept {
    return static_cast<ExecStateFlag>(_execState.cnd->load(_STD memory_order::consume)) == ExecStateFlag::eDone;
}

void Stage::reset() {
    _execState.state.store(
        static_cast<u8>(ExecStateFlag::eUndefined),
        /* Should be `_STD memory_order::release` */
        _STD memory_order::relaxed
    );
}

bool Stage::start() {
    u8 expect { static_cast<u8>(ExecStateFlag::eUndefined) };
    return _execState.state.compare_exchange_strong(
        expect,
        static_cast<u8>(ExecStateFlag::eRunning),
        _STD memory_order::acq_rel
    );
}

void Stage::finish() {
    _execState.state.store(static_cast<u8>(ExecStateFlag::eDone), _STD memory_order::release);
}
