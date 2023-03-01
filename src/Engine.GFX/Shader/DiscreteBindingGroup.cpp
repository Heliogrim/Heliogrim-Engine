#include "DiscreteBindingGroup.hpp"

using namespace hg::engine::gfx::shader;
using namespace hg;

DiscreteBindingGroup::DiscreteBindingGroup(
    cref<ShaderBindingGroup> super_,
    const ptr<DiscreteBindingPool> pool_,
    cref<vk::DescriptorSet> vkSet_
) :
    _super(super_),
    _pool(pool_),
    _discreteBindings(),
    _vkSet(vkSet_) {}

ref<DiscreteBindingGroup::this_type> DiscreteBindingGroup::operator=(mref<this_type> other_) noexcept {

    if (_STD addressof(other_) != this) {
        _super = _STD move(other_._super);
        _pool = _STD move(other_._pool);
        _discreteBindings = _STD move(other_._discreteBindings);
        _vkSet = _STD move(other_._vkSet);
    }

    return *this;
}

cref<ShaderBindingGroup> DiscreteBindingGroup::super() const noexcept {
    return _super;
}

const ptr<engine::gfx::DiscreteBindingPool> DiscreteBindingGroup::pool() const noexcept {
    return _pool;
}

ref<DiscreteBinding> DiscreteBindingGroup::getById(const ShaderBinding::id_type id_) {

    const auto it {
        _STD find_if(
            _discreteBindings.begin(),
            _discreteBindings.end(),
            [id_](cref<DiscreteBinding> entry_) {
                return id_ == entry_.super().id();
            }
        )
    };

    if (it != _discreteBindings.end()) {
        return *it;
    }

    /**
     *
     */
    _discreteBindings.push_back(
        DiscreteBinding {
            &_super.getById(id_),
            _vkSet
        }
    );

    /**
     *
     */
    return getById(id_);
}

cref<vk::DescriptorSet> DiscreteBindingGroup::vkSet() const noexcept {
    return _vkSet;
}

DiscreteBindingGroup::operator bool() const noexcept {
    return _vkSet.operator bool();
}
