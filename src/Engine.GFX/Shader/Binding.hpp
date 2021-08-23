#pragma once

#include <Engine.Common/Types.hpp>
#include "Type.hpp"
#include "../Buffer/Buffer.hpp"
#include "../Device/Device.hpp"
#include "../Texture/Texture.hpp"
#include "../Texture/TextureSampler.hpp"

namespace ember::engine::gfx::shader {
    class Binding {
    public:
        using id_type = u32;

        using value_type = Binding;
        using reference_type = Binding&;
        using const_reference_type = const Binding&;

        /**
         * Default constructor
         *
         * @author Julius
         * @date 12.11.2020
         */
        Binding() = delete;

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
         * @param  vkSet_ Set the vk belongs to.
         * @param  vkSetLayout_ The vk set layout.
         */
        Binding(const BindingType type_, const id_type id_, const BindingUpdateInterval interval_,
            cref<sptr<Device>> device_, const vk::DescriptorSet& vkSet_,
            const vk::DescriptorSetLayout& vkSetLayout_) noexcept;

        /**
         * Destructor
         *
         * @author Julius
         * @date 12.11.2020
         */
        ~Binding() = default;

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
        vk::DescriptorSet _vkSet;
        vk::DescriptorSetLayout _vkSetLayout;

    public:
        [[nodiscard]] vk::DescriptorSet vkSet() const noexcept;

        [[nodiscard]] vk::DescriptorSetLayout vkSetLayout() const noexcept;

    public:
        /**
         * Stores the given buffer
         *
         * @author Julius
         * @date 21.11.2020
         *
         * @param  buffer_ The buffer to store.
         */
        void store(const Buffer& buffer_);

    private:
        /**
         * 
         */
        TextureSampler _sampler;

    public:
        /**
         * Sets a sampler
         *
         * @author Julius
         * @date 14.12.2020
         *
         * @param  sampler_ The sampler.
         */
        void setSampler(const TextureSampler& sampler_) noexcept;

        /**
         * Stores the given texture
         *
         * @author Julius
         * @date 21.11.2020
         *
         * @param  texture_ The texture to store.
         */
        void store(const Texture& texture_);
    };
}
