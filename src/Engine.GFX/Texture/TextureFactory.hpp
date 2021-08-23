#pragma once

#include <Engine.Common/Wrapper.hpp>
#include "Texture.hpp"
#include "VirtualTexture.hpp"
#include "../Device/Device.hpp"

namespace ember::engine::gfx {

    struct TextureBuildPayload {

        /**
         * Extent ( Width | Height | Depth )
         */
        math::uivec3 extent = { 0ui32 };

        /**
         * Format
         */
        TextureFormat format = TextureFormat::eUndefined;

        /**
         * Mip Level
         */
        u32 mipLevels = { 1 };

        /**
         * Type
         */
        TextureType type = TextureType::eUndefined;

        /**
         * Vulkan API
         */
        vk::ImageAspectFlags vkAspect;
        vk::ImageUsageFlags vkUsage;

        vk::MemoryPropertyFlags vkMemoryFlags;
        vk::SharingMode vkSharing;

        /**
         * Cast that converts the given to a bool
         *
         * @author Julius
         * @date 07.12.2020
         *
         * @returns The result of the operation.
         */
        operator bool() const;
    };

    struct VirtualTextureBuildPayload {
        /**
         * Cast that converts the given  to a bool
         *
         * @author Julius
         * @date 07.12.2020
         *
         * @returns The result of the operation.
         */
        operator bool() const;
    };

    class TextureFactory {
    public:

        /**
         * Gets the singleton instance
         *
         * @author Julius
         * @date 14.12.2020
         *
         * @returns A ptr&lt;Ty&gt;
         */
        [[nodiscard]] static ptr<TextureFactory> get() {
            return TextureFactory::_instance;
        }

        /**
         * Destroys this
         *
         * @author Julius
         * @date 04.12.2020
         */
        [[nodiscard]] static void destroy();

    protected:
        /**
         * Singleton Instance
         */
        static ptr<TextureFactory> _instance;

        /**
         * Constructor
         *
         * @author Julius
         * @date 04.12.2020
         *
         * @param  device_ The device.
         */
        TextureFactory(cref<sptr<Device>> device_);

        /**
         * Destructor
         *
         * @author Julius
         * @date 04.12.2020
         */
        virtual ~TextureFactory() noexcept;

    protected:
        /**
         * Device
         */
        sptr<Device> _device;

    public:
        /**
         * Gets the device
         *
         * @author Julius
         * @date 04.12.2020
         *
         * @returns A const.
         */
        [[nodiscard]] cref<sptr<Device>> device() const noexcept;

    public:
        /**
         * Builds a Texture
         *
         * @author Julius
         * @date 04.12.2020
         *
         * @param  payload_ The payload.
         *
         * @returns A Texture.
         */
        [[nodiscard]] virtual Texture build(const TextureBuildPayload& payload_) const = 0;

        /**
         * Builds internal Texture View
         *
         * @author Julius
         * @date 14.12.2020
         *
         * @param [in,out] texture_ The texture.
         *
         * @returns A reference to a Texture.
         */
        virtual Texture& buildView(Texture& texture_) const = 0;

        /**
         * Builds a virtual Texture
         *
         * @author Julius
         * @date 04.12.2020
         *
         * @param  type_ The type.
         *
         * @returns A Texture.
         */
        [[nodiscard]] virtual VirtualTexture buildVirtual(const TextureType type_) const = 0;
    };
}
