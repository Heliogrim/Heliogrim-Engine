#pragma once

#include "ShaderBinding.hpp"
#include "ShaderBindingGroupLayout.hpp"

namespace ember::engine::gfx {
    /**
     * Forward Declaration
     */
    class DiscreteBindingPool;
}

namespace ember::engine::gfx::shader {
    class DiscreteBindingGroup;
}

namespace ember::engine::gfx::shader {

    class ShaderBindingGroup {
    public:
        /**
         * Constructor
         *
         * @author Julius
         * @date 30.11.2020
         *
         * @param  interval_ The interval.
         * @param  layout_ The layout of bindings.
         * @param  vkSetLayout_ The vk set layout.
         */
        ShaderBindingGroup(
            const BindingUpdateInterval interval_,
            mref<ShaderBindingGroupLayout> layout_,
            const vk::DescriptorSetLayout vkSetLayout_
        ) noexcept;

    private:
        /**
         * Update Interval
         */
        BindingUpdateInterval _interval;

    public:
        /**
         * Gets the interval for binding updates
         *
         * @author Julius
         * @date 29.11.2020
         *
         * @returns A BindingUpdateInterval.
         */
        [[nodiscard]] BindingUpdateInterval interval() const noexcept;

    private:
        /**
         * Shader Bindings 
         */
        Vector<ShaderBinding> _shaderBindings;

    public:
        /**
         * Gets the shader bindings
         *
         * @author Julius
         * @date 29.11.2020
         *
         * @returns A list of.
         */
        [[nodiscard]] const Vector<ShaderBinding>& shaderBindings() const noexcept;

        /**
         * Adds binding_
         *
         * @author Julius
         * @date 29.11.2020
         *
         * @param  binding_ The binding to add.
         */
        void add(const ShaderBinding& binding_);

        /**
         * Determine if 'id_' exists
         *
         * @author Julius
         * @date 29.11.2020
         *
         * @param  id_ The identifier.
         *
         * @returns True if it succeeds, false if it fails.
         */
        [[nodiscard]] bool exists(const ShaderBinding::id_type id_) const noexcept;

        /**
         * Gets by identifier
         *
         * @author Julius
         * @date 29.11.2020
         *
         * @param  id_ The identifier.
         *
         * @returns The by identifier.
         */
        [[nodiscard]] ShaderBinding& getById(const ShaderBinding::id_type id_);

        [[nodiscard]] cref<ShaderBinding> getById(const ShaderBinding::id_type id_) const;

    private:
        ShaderBindingGroupLayout _layout;

    public:
        /**
         * Gets the layout of this shader binding group
         *
         * @author Julius
         * @date 28.04.2022
         *
         * @returns A const reference to the ShaderBindingGroupLayout.
         */
        [[nodiscard]] cref<ShaderBindingGroupLayout> layout() const noexcept;

    private:
        /**
         * Descriptor Set Layout
         */
        vk::DescriptorSetLayout _vkSetLayout;

    public:
        /**
         * Vk set layout
         *
         * @author Julius
         * @date 29.11.2020
         *
         * @returns A vk::DescriptorSetLayout.
         */
        [[nodiscard]] cref<vk::DescriptorSetLayout> vkSetLayout() const noexcept;

    public:
        [[nodiscard]] DiscreteBindingGroup useDiscrete(cref<vk::DescriptorSet> vkSet_) const noexcept;

        [[nodiscard]] DiscreteBindingGroup useDiscrete(cref<vk::DescriptorSet> vkSet_,
            const ptr<DiscreteBindingPool> pool_) const noexcept;
    };
}
