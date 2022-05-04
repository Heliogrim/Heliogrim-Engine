#include "DiscreteBindingPool.hpp"

#include "../Device/Device.hpp"

#ifdef _DEBUG
#include <cassert>
#endif

using namespace ember::engine::gfx;
using namespace ember;

DiscreteBindingPool::DiscreteBindingPool(
    cref<sptr<Device>> device_,
    cref<shader::ShaderBindingGroupLayout> layout_,
    const u32 sets_
) noexcept :
    _device(device_),
    _layout(layout_),
    _maxSets(sets_) {
    setup();
}

DiscreteBindingPool::~DiscreteBindingPool() {
    tidy();
}

void DiscreteBindingPool::tidy() {

    if (_vkPool) {
        _device->vkDevice().resetDescriptorPool(_vkPool, vk::DescriptorPoolResetFlags {});
        _device->vkDevice().destroyDescriptorPool(_vkPool);
    }
}

void DiscreteBindingPool::setup() {

    Vector<vk::DescriptorPoolSize> sizes {};

    if (_layout.images > 0ui32) {
        sizes.push_back(vk::DescriptorPoolSize {
            vk::DescriptorType::eCombinedImageSampler,
            _maxSets * _layout.images
        });
    }

    if (_layout.uniforms > 0ui32) {
        sizes.push_back(vk::DescriptorPoolSize {
            vk::DescriptorType::eUniformBuffer,
            _maxSets * _layout.uniforms
        });
    }

    if (_layout.storages > 0ui32) {
        sizes.push_back(vk::DescriptorPoolSize {
            vk::DescriptorType::eStorageBuffer,
            _maxSets * _layout.storages
        });
    }

    const vk::DescriptorPoolCreateInfo pci {
        vk::DescriptorPoolCreateFlagBits::eUpdateAfterBind,
        _maxSets,
        UINT32_T(sizes.size()),
        sizes.data()
    };
    const auto result { _device->vkDevice().createDescriptorPool(pci) };
    #ifdef _DEBUG
    assert(result);
    #endif

    _vkPool = result;
}

cref<shader::ShaderBindingGroupLayout> DiscreteBindingPool::layout() const noexcept {
    return _layout;
}

u32 DiscreteBindingPool::maxSets() const noexcept {
    return _maxSets;
}

u32 DiscreteBindingPool::leftSets() const noexcept {
    return _maxSets - _STD min(_maxSets, _currentSets);
}

bool DiscreteBindingPool::exceeded() const noexcept {
    return _exceeded;
}

void DiscreteBindingPool::markAsExceeded() noexcept {
    _exceeded = true;
}

cref<vk::DescriptorPool> DiscreteBindingPool::vkPool() const noexcept {
    return _vkPool;
}

shader::DiscreteBindingGroup DiscreteBindingPool::allocate(cref<shader::ShaderBindingGroup> group_) {
    #ifdef _DEBUG
    // Validate ShaderBindingGroup is compatible with layout
    shader::ShaderBindingGroupLayout testLayout {};
    for (const auto& entry : group_.shaderBindings()) {
        switch (entry.type()) {
            case shader::BindingType::eStorageBuffer: {
                ++testLayout.storages;
                break;
            }
            case shader::BindingType::eUniformBuffer: {
                ++testLayout.uniforms;
                break;
            }
            case shader::BindingType::eImageSampler: {
                ++testLayout.images;
                break;
            }
            default: {
                __debugbreak();
            }
        }
    }
    assert(_layout == testLayout);
    #endif

    const vk::DescriptorSetAllocateInfo ai { _vkPool, 1, &group_.vkSetLayout() };
    const auto result { _device->vkDevice().allocateDescriptorSets(ai) };
    #ifdef _DEBUG
    assert(result.size() == 1);
    #endif

    ++_currentSets;
    return group_.useDiscrete(result.front(), this);
}

void DiscreteBindingPool::free(mref<shader::DiscreteBindingGroup> group_) {
    _device->vkDevice().freeDescriptorSets(_vkPool, 1, &group_.vkSet());
    --_currentSets;
}
