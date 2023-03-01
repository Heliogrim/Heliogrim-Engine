#include "StyleKeyStack.hpp"

#include <cassert>

using namespace hg::engine::reflow;
using namespace hg;

StyleKeyStack::StyleKeyStack() :
    _stack(),
    _depth() {}

StyleKeyStack::~StyleKeyStack() noexcept = default;

cref<Vector<StyleKeyStack::stack_value_type>> StyleKeyStack::getSetList() const noexcept {
    return _stack._Get_container();
}

bool StyleKeyStack::contains(cref<key_type> key_) const noexcept {
    for (const auto& kv : getSetList()) {
        if (kv.second.contains(key_)) {
            return true;
        }
    }

    return false;
}

void StyleKeyStack::pushKey(cref<key_type> key_) {
    assert(not _stack.empty());

    _stack.top().second.insert(key_);
}

void StyleKeyStack::popKey(cref<key_type> key_) {
    assert(not _stack.empty());

    _stack.top().second.erase(key_);
}

bool StyleKeyStack::pushLayer() {

    ++_depth;

    if (not _stack.empty() && _stack.top().second.empty()) {
        _stack.top().first = _depth;
        return false;
    }

    _stack.push(_STD make_pair(_depth, set_type {}));
    return true;
}

bool StyleKeyStack::popLayer() {

    --_depth;

    if (not _stack.empty() && _stack.top().first > _depth) {
        _stack.pop();
        return true;
    }

    return false;
}

void StyleKeyStack::compress(ref<set_type> dst_) const noexcept {
    for (const auto& kv : getSetList()) {
        dst_.insert(kv.second.begin(), kv.second.end());
    }
}
