#pragma once
#include "ShaderBindingGroup.hpp"
#include "DiscreteBinding.hpp"

namespace ember::engine::gfx::shader {

    class DiscreteBindingGroup {
    public:
        using this_type = DiscreteBindingGroup;

    public:
        DiscreteBindingGroup(cref<ShaderBindingGroup> super_, cref<vk::DescriptorSet> vkSet_);

        DiscreteBindingGroup(cref<this_type>) = delete;

        DiscreteBindingGroup(mref<this_type>) noexcept = default;

        ~DiscreteBindingGroup() = default;

    private:
        ShaderBindingGroup _super;

    public:
        [[nodiscard]] cref<ShaderBindingGroup> super() const noexcept;

    private:
        /**
         * Discrete Bindings
         */
        Vector<DiscreteBinding> _discreteBindings;

    public:
        /**
         * Get a discrete binding by the shader binding's identifier
         *
         * @author Julius
         * @date 17.02.2022
         *
         * @param id_ The shader binding identifier.
         *
         * @details No need to store DiscreteBinding's as set of data, cause it'S just the structured pairing of the groups descriptor and shader binding itself.
         *
         * @returns A reference to the discrete binding instance.
         */
        [[nodiscard]] ref<DiscreteBinding> getById(const ShaderBinding::id_type id_);

    private:
        vk::DescriptorSet _vkSet;

    public:
        [[nodiscard]] cref<vk::DescriptorSet> vkSet() const noexcept;
    };
}
