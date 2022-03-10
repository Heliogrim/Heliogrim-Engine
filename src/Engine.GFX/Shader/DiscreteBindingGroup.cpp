#include "DiscreteBindingGroup.hpp"

using namespace ember::engine::gfx::shader;
using namespace ember;

DiscreteBindingGroup::DiscreteBindingGroup(cref<ShaderBindingGroup> super_, cref<vk::DescriptorSet> vkSet_) :
    _super(super_),
    _discreteBindings(),
    _vkSet(vkSet_) {}

cref<ShaderBindingGroup> DiscreteBindingGroup::super() const noexcept {
    return _super;
}

ref<DiscreteBinding> DiscreteBindingGroup::getById(const ShaderBinding::id_type id_) {

    const auto it {
        _STD find_if(_discreteBindings.begin(), _discreteBindings.end(), [id_](cref<DiscreteBinding> entry_) {
            return id_ == entry_.super().id();
        })
    };

    if (it != _discreteBindings.end()) {
        return *it;
    }

    /**
     *
     */
    _discreteBindings.push_back(DiscreteBinding {
        &_super.getById(id_),
        _vkSet
    });

    /**
     *
     */
    return getById(id_);
}

cref<vk::DescriptorSet> DiscreteBindingGroup::vkSet() const noexcept {
    return _vkSet;
}
