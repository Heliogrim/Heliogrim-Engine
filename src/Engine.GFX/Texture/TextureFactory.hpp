#pragma once

#include <Engine.Common/Wrapper.hpp>
#include "Texture.hpp"
#include "SparseTexture.hpp"
#include "../Device/Device.hpp"

namespace hg::engine::gfx {
    struct TextureBuildPayload {
        /**
         * Extent ( Width | Height | Depth )
         */
        math::uivec3 extent { 0uL };

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

    struct SparseTextureBuildPayload {
        /**
         * Number of layers
         */
        u32 layers;

        /**
         * Extent ( Width | Height | Depth )
         */
        math::uivec3 extent;

        /**
         * Format
         */
        TextureFormat format = TextureFormat::eUndefined;

        /**
         * Mip Levels to use [min, max)
         */
        math::uivec2 mipLevels;

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
         * Cast that converts the given  to a bool
         *
         * @author Julius
         * @date 07.12.2020
         *
         * @returns The result of the operation.
         */
        operator bool() const;
    };

    struct TextureViewBuildOptions {
        /**
         * Range of layers `[min, max)`
         */
        math::ivec2 layers = { -1L, -1L };

        /**
         * Texture View Type
         *
         * @note TextureType used for the internal view has to be compatible with the underlying texture type
         *
         *  | Texture Type  | Compatible View Types
         *  | ------------- | ---------------------
         *  | eUndefined    | `-`
         *  | e2d           | e2d
         *  | e2dArray      | e2d, e2dArray
         *  | e3d           | e3d
         *  | eCube         | eCube
         * 
         */
        TextureType type = TextureType::eUndefined;

        /**
         * Range of mip levels `[min, max)`
         */
        math::ivec2 mipLevels = { -1L, -1L };

        /*
         * TODO: Check for support:
         *  - sub-planar slices
         *  - sub-resource range texture format
         *  - multi-aspect textures
         *
         * TODO: Check for requirement:
         *  - view dependent component swizzle
         */
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
        static void destroy();

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
        [[nodiscard]] virtual Texture build(
            const TextureBuildPayload& payload_
        ) const = 0;

        /**
         * Builds internal Texture View
         *
         * @author Julius
         * @date 14.12.2020
         *
         * @param [in,out] texture_ The texture.
         * @param options_ The options to take into account ( default will use auto-deduction from texture )
         *
         * @returns A reference to a Texture.
         */
        virtual Texture& buildView(
            Texture& texture_,
            TextureViewBuildOptions options_ = {}
        ) const = 0;

        virtual ref<TextureView> buildView(
            ref<TextureView> texture_,
            TextureViewBuildOptions options_ = {}
        ) const = 0;

        virtual ref<SparseTexture> buildView(
            ref<SparseTexture> texture_,
            TextureViewBuildOptions options_ = {}
        ) const = 0;

        virtual ref<SparseTextureView> buildView(
            ref<SparseTextureView> texture_,
            TextureViewBuildOptions options_ = {}
        ) const = 0;

        /**
         * Builds a virtual Texture
         *
         * @author Julius
         * @date 04.12.2020
         *
         * @param  payload_ The payload.
         *
         * @returns A Texture.
         */
        [[nodiscard]] virtual ptr<SparseTexture> buildVirtual(
            const SparseTextureBuildPayload& payload_
        ) const = 0;
    };
}
