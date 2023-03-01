#include "ShaderBinding.hpp"

#include "DiscreteBinding.hpp"

using namespace hg::engine::gfx::shader;
using namespace hg::engine::gfx;
using namespace hg;

ShaderBinding::ShaderBinding(
    const BindingType type_,
    const id_type id_,
    const BindingUpdateInterval interval_,
    cref<sptr<Device>> device_,
    const vk::DescriptorSetLayout& vkSetLayout_
) noexcept :
    _type(type_),
    _id(id_),
    _interval(interval_),
    _device(device_),
    _vkSetLayout(vkSetLayout_) {}

BindingType ShaderBinding::type() const noexcept {
    return _type;
}

u32 ShaderBinding::id() const {
    return _id;
}

BindingUpdateInterval ShaderBinding::interval() const noexcept {
    return _interval;
}

cref<sptr<Device>> ShaderBinding::device() const noexcept {
    return _device;
}

vk::DescriptorSetLayout ShaderBinding::vkSetLayout() const noexcept {
    return _vkSetLayout;
}

DiscreteBinding ShaderBinding::useDiscrete(cref<vk::DescriptorSet> vkSet_) const noexcept {
    return DiscreteBinding {
        this,
        vkSet_
    };
}
