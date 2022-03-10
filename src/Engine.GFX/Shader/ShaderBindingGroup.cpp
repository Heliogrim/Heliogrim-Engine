#include "ShaderBindingGroup.hpp"

#include "DiscreteBindingGroup.hpp"

using namespace ember::engine::gfx::shader;
using namespace ember::engine::gfx;
using namespace ember;

ShaderBindingGroup::ShaderBindingGroup(const BindingUpdateInterval interval_,
    const vk::DescriptorSetLayout vkSetLayout_) noexcept :
    _interval(interval_),
    _vkSetLayout(vkSetLayout_) {}

BindingUpdateInterval ShaderBindingGroup::interval() const noexcept {
    return _interval;
}

const Vector<ShaderBinding>& ShaderBindingGroup::shaderBindings() const noexcept {
    return _shaderBindings;
}

void ShaderBindingGroup::add(const ShaderBinding& binding_) {
    #if _DEBUG
    assert(
        !exists(binding_.id()) &&
        binding_.vkSetLayout() == _vkSetLayout
    );
    #endif

    _shaderBindings.push_back(binding_);
}

bool ShaderBindingGroup::exists(const ShaderBinding::id_type id_) const noexcept {
    return _STD find_if(_shaderBindings.begin(), _shaderBindings.end(), [&id_](const ShaderBinding& entry_) {
        return entry_.id() == id_;
    }) != _shaderBindings.end();
}

ShaderBinding& ShaderBindingGroup::getById(const ShaderBinding::id_type id_) {
    return *(
        _STD find_if(_shaderBindings.begin(), _shaderBindings.end(), [&id_](const ShaderBinding& entry_) {
            return entry_.id() == id_;
        })
    );
}

cref<ShaderBinding> ShaderBindingGroup::getById(const ShaderBinding::id_type id_) const {
    return const_cast<ShaderBindingGroup*>(this)->getById(id_);
}

cref<vk::DescriptorSetLayout> ShaderBindingGroup::vkSetLayout() const noexcept {
    return _vkSetLayout;
}

DiscreteBindingGroup ShaderBindingGroup::useDiscrete(cref<vk::DescriptorSet> vkSet_) const noexcept {
    return DiscreteBindingGroup {
        *this,
        vkSet_
    };
}
