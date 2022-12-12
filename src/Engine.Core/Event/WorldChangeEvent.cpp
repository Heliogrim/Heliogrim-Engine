#include "WorldChangeEvent.hpp"

using namespace ember::engine::core;
using namespace ember;

WorldChangeEvent::WorldChangeEvent(
    const non_owning_rptr<Session> session_,
    cref<sptr<World>> prevWorld_,
    cref<sptr<World>> nextWorld_
) :
    StatelessEvent(),
    _session(session_),
    _prevWorld(prevWorld_),
    _nextWorld(nextWorld_) {}

const non_owning_rptr<Session> WorldChangeEvent::getSession() const noexcept {
    return _session;
}

cref<sptr<World>> WorldChangeEvent::getPrevWorld() const noexcept {
    return _prevWorld;
}

cref<sptr<World>> WorldChangeEvent::getNextWorld() const noexcept {
    return _nextWorld;
}
