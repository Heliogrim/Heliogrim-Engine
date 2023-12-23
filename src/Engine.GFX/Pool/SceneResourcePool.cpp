#include "SceneResourcePool.hpp"

using namespace hg::engine::gfx;
using namespace hg;

SceneResourcePool::SceneResourcePool(cref<sptr<Device>> device_) :
    _device(device_),
    _scene(nullptr),
    staticInstancePool(device_),
    staticAabbPool(_device),
    lightSourcePool(_device) {}

SceneResourcePool::~SceneResourcePool() = default;

void SceneResourcePool::setup() {
    staticInstancePool.setup(128uL);
    staticAabbPool.setup(128uL);
    lightSourcePool.setup(32uL);
}

void SceneResourcePool::destroy() {
    lightSourcePool.destroy();
    staticAabbPool.destroy();
    staticInstancePool.destroy();
}

sptr<Device> SceneResourcePool::device() const noexcept {
    return _device;
}

nmpt<engine::scene::Scene> SceneResourcePool::scene() const noexcept {
    return _scene;
}
