#include "SceneResourcePool.hpp"

#include "../Buffer/BufferFactory.hpp"
#include "../Texture/TextureFactory.hpp"

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

    const auto bufferFactory = BufferFactory::get();
    const auto textureFactory = TextureFactory::get();

    staticInstancePool.setup(128uL);
    staticAabbPool.setup(128uL);

    sceneLightInfo = { 0uL };
    sceneLightInfoBuffer = bufferFactory->build(
        {
            sizeof(GlslSceneLightInfo),
            alignof(GlslSceneLightInfo),
            MemoryProperty::eHostVisible,
            vk::BufferCreateFlags {},
            vk::BufferUsageFlagBits::eUniformBuffer
        }
    );
    lightSourcePool.setup(32uL);
}

void SceneResourcePool::destroy() {
    lightSourcePool.destroy();
    sceneLightInfoBuffer.destroy();

    staticAabbPool.destroy();
    staticInstancePool.destroy();
}

sptr<Device> SceneResourcePool::device() const noexcept {
    return _device;
}

nmpt<engine::scene::Scene> SceneResourcePool::scene() const noexcept {
    return _scene;
}
