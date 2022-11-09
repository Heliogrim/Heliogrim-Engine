#include "WindowManager.hpp"

using namespace ember::engine::reflow;
using namespace ember;

sptr<WindowManager> WindowManager::_instance = nullptr;

cref<sptr<WindowManager::this_type>> WindowManager::get() {
    return _instance;
}

cref<sptr<WindowManager::this_type>> WindowManager::make() {
    if (!_instance) {
        _instance = sptr<WindowManager>(new WindowManager());
    }
    return _instance;
}

void WindowManager::destroy() {
    _instance.reset();
}

WindowManager::WindowManager() {}

WindowManager::~WindowManager() = default;
