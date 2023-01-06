#include "Stage.hpp"

using namespace ember::engine::scheduler;
using namespace ember;

Stage::Stage(cref<string> identifier_, const s8 slot_) noexcept :
    _refCount(1ui16),
    _identifier(identifier_),
    _slot(slot_) {}

void Stage::incRef() noexcept {
    ++_refCount;
}

bool Stage::decRef() noexcept {
    return (--_refCount) == 0;
}

cref<string> Stage::getIdentifier() const noexcept {
    return _identifier;
}

s8 Stage::getSlot() const noexcept {
    return _slot;
}
