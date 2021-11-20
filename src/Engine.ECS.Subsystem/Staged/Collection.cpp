#include "Collection.hpp"

using namespace ember::engine::ecs::subsystem::staged;
using namespace ember;

void Collection::submit(mref<Submit> submit_) {
    _submits.push_back(submit_);
}

void Collection::reset() {
    _submits.clear();
}

vector<Submit>::iterator Collection::begin() noexcept {
    return _submits.begin();
}

vector<Submit>::iterator Collection::end() noexcept {
    return _submits.end();
}
