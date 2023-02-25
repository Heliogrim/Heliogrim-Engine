#pragma once

#include <Engine.Common/Types.hpp>
#include "Type.hpp"
#include "../Device/Device.hpp"

namespace ember::engine::gfx::shader {
    /**
     * Forward Decalaration
     */
    class DiscreteBinding;

    class ShaderBinding {
    public:
        using id_type = u32;

        using value_type = ShaderBinding;
        using reference_type = ShaderBinding&;
        using const_reference_type = const ShaderBinding&;

        /**
         * Default constructor
         *
         * @author Julius
         * @date 12.11.2020
         */
        ShaderBinding() = delete;

        /**
         * Constructor
         *
         * @author Julius
         * @date 27.11.2020
         *
         * @param  type_ The type.
         * @param  id_ The identifier.
         * @param  interval_ The interval.
         * @param  device_ The device.
         * @param  vkSetLayout_ The vk set layout.
         */
        ShaderBinding(
            const BindingType type_,
            const id_type id_,
            const BindingUpdateInterval interval_,
            cref<sptr<Device>> device_,
            const vk::DescriptorSetLayout& vkSetLayout_
        ) noexcept;

        /**
         * Destructor
         *
         * @author Julius
         * @date 12.11.2020
         */
        ~ShaderBinding() = default;

    private:
        /**
         * Type
         */
        BindingType _type;

    public:
        /**
         * Gets the type
         *
         * @author Julius
         * @date 27.11.2020
         *
         * @returns A BindingType.
         */
        [[nodiscard]] BindingType type() const noexcept;

    private:
        /**
         * The identifier
         */
        id_type _id;

    public:
        /**
         * Gets the identifier
         *
         * @author Julius
         * @date 27.11.2020
         *
         * @returns An u32.
         */
        [[nodiscard]] u32 id() const;

    private:
        /**
         * Update Interval
         */
        BindingUpdateInterval _interval;

    public:
        /**
         * Gets the binding update interval
         *
         * @author Julius
         * @date 29.11.2020
         *
         * @returns A BindingUpdateInterval.
         */
        [[nodiscard]] BindingUpdateInterval interval() const noexcept;

    private:
        /**
         * Device
         */
        sptr<Device> _device;

    public:
        /**
         * Gets the device
         *
         * @author Julius
         * @date 27.11.2020
         *
         * @returns A const.
         */
        [[nodiscard]] cref<sptr<Device>> device() const noexcept;

    private:
        /**
         * 
         */
        vk::DescriptorSetLayout _vkSetLayout;

    public:
        [[nodiscard]] vk::DescriptorSetLayout vkSetLayout() const noexcept;

    public:
        [[nodiscard]] DiscreteBinding useDiscrete(cref<vk::DescriptorSet> vkSet_) const noexcept;
    };
}
