#include "VkDescriptor.hpp"

#include <Engine.Core/Engine.hpp>
#include <Engine.GFX/Graphics.hpp>

using namespace hg::driver::vk;
using namespace hg;

driver::vk::VkDescriptorPool::VkDescriptorPool() noexcept :
    pooled(),
    vkPool(nullptr) {}

driver::vk::VkDescriptorPool::VkDescriptorPool(mref<this_type> other_) noexcept :
    pooled(_STD move(other_.pooled)),
    vkPool(_STD exchange(other_.vkPool, nullptr)) {}

driver::vk::VkDescriptorPool::~VkDescriptorPool() noexcept {

    if (vkPool == nullptr) {
        return;
    }

    const auto device = engine::Engine::getEngine()->getGraphics()->getCurrentDevice();
    device->vkDevice().destroyDescriptorPool(reinterpret_cast<::VkDescriptorPool>(vkPool));
    vkPool = nullptr;
}
