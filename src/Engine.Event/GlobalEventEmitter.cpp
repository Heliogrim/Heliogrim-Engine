#include "GlobalEventEmitter.hpp"

using namespace ember;

ptr<GlobalEventEmitter> GlobalEventEmitter::_instance = nullptr;

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

const ptr<GlobalEventEmitter::value_type> GlobalEventEmitter::get() noexcept {
    return _instance;
}

const ptr<GlobalEventEmitter::value_type> GlobalEventEmitter::make() noexcept {

    if (!_instance) {
        _instance = new GlobalEventEmitter();
    }

    return _instance;
}

void GlobalEventEmitter::destroy() noexcept {
    delete _instance;
    _instance = nullptr;
}
