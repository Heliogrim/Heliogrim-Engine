#include "GlobalEventEmitter.hpp"

using namespace ember;

GlobalEventEmitter::GlobalEventEmitter() noexcept :
    _emitterMtx(),
    _emitter() {}

GlobalEventEmitter::~GlobalEventEmitter() noexcept {
    tidy();
}

void GlobalEventEmitter::tidy() noexcept {
    for (auto entry : _emitter) {
        delete entry.second;
        entry.second = nullptr;
    }

    _emitter.clear();
}
