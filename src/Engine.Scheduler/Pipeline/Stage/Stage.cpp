#include "Stage.hpp"

using namespace hg::engine::scheduler;
using namespace hg;

Stage::Stage(mref<string> identifier_, const s8 slot_) noexcept :
    _refCount(1ui16),
    _identifier(std::move(identifier_)),
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
