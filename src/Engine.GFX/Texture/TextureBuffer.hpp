#pragma once

#include <Engine.Common/Types.hpp>
#include "../vkinc.hpp"

namespace ember::engine::gfx {

    class TextureBuffer final {
    public:
        /**
         * Default constructor
         *
         * @author Julius
         * @date 20.11.2020
         */
        TextureBuffer();

        /**
         * Destructor
         *
         * @author Julius
         * @date 20.11.2020
         */
        ~TextureBuffer() noexcept;

        /**
         * Cast that converts the given to a bool
         *
         * @author Julius
         * @date 20.12.2020
         *
         * @returns The result of the operation.
         */
        operator bool() const noexcept;

        /**
         * Destroys this
         *
         * @author Julius
         * @date 25.11.2020
         */
        void destroy();

    private:
        /**
         * Device
         */
        vk::Device _vkDevice;

    public:
        /**
         * Gets the device
         *
         * @author Julius
         * @date 20.12.2020
         *
         * @returns A reference to a vk::Device.
         */
        [[nodiscard]] vk::Device& vkDevice() noexcept;

    private:
        /**
         * Image
         */
        vk::Image _image;

    public:
        /**
         * Gets the image
         *
         * @author Julius
         * @date 20.11.2020
         *
         * @returns A vk::Image.
         */
        [[nodiscard]] vk::Image image() const noexcept;

        /**
         * Gets the image
         *
         * @author Julius
         * @date 07.12.2020
         *
         * @returns A reference to a vk::Image.
         */
        [[nodiscard]] vk::Image& image() noexcept;

    private:
        /**
         * Memory
         */
        vk::DeviceMemory _memory;

    public:
        /**
         * Gets the memory
         *
         * @author Julius
         * @date 20.11.2020
         *
         * @returns A vk::DeviceMemory.
         */
        [[nodiscard]] vk::DeviceMemory memory() const noexcept;

        /**
         * Gets the memory
         *
         * @author Julius
         * @date 07.12.2020
         *
         * @returns A reference to a vk::DeviceMemory.
         */
        [[nodiscard]] vk::DeviceMemory& memory() noexcept;

    private:
        /**
         * Memory Size
         */
        u64 _memorySize;

    public:
        /**
         * Memory size
         *
         * @author Julius
         * @date 20.11.2020
         *
         * @returns An u64.
         */
        [[nodiscard]] u64 memorySize() const noexcept;

        /**
         * Memory size
         *
         * @author Julius
         * @date 07.12.2020
         *
         * @returns A reference to an u64.
         */
        [[nodiscard]] u64& memorySize() noexcept;

    private:
        /**
         * Memory Align
         */
        u64 _memoryAlign;

    public:
        /**
         * Memory align
         *
         * @author Julius
         * @date 20.11.2020
         *
         * @returns An u64.
         */
        [[nodiscard]] u64 memoryAlign() const noexcept;

        /**
         * Memory align
         *
         * @author Julius
         * @date 07.12.2020
         *
         * @returns A reference to an u64.
         */
        [[nodiscard]] u64& memoryAlign() noexcept;

    private:
        /**
         * Memory Mapped
         */
        void* _memoryMapped;

    public:
        /**
         * Memory mapped
         *
         * @author Julius
         * @date 21.12.2020
         *
         * @returns Null if it fails, else a pointer to a void.
         */
        [[nodiscard]] void* memoryMapped() const noexcept;

        [[nodiscard]] void*& memoryMapped() noexcept;

    private:
        /**
         * Sharing Mode
         */
        vk::SharingMode _sharing;

    public:
        /**
         * Sharing mode
         *
         * @author Julius
         * @date 20.11.2020
         *
         * @returns A vk::SharingMode.
         */
        [[nodiscard]] vk::SharingMode sharingMode() const noexcept;

        /**
         * Sharing mode
         *
         * @author Julius
         * @date 07.12.2020
         *
         * @returns A reference to a vk::SharingMode.
         */
        [[nodiscard]] vk::SharingMode& sharingMode() noexcept;

    private:
        /**
         * Tiling
         */
        vk::ImageTiling _tiling;

    public:
        /**
         * Gets the tiling
         *
         * @author Julius
         * @date 20.11.2020
         *
         * @returns A vk::ImageTiling.
         */
        [[nodiscard]] vk::ImageTiling tiling() const noexcept;

        /**
         * Gets the tiling
         *
         * @author Julius
         * @date 07.12.2020
         *
         * @returns A reference to a vk::ImageTiling.
         */
        [[nodiscard]] vk::ImageTiling& tiling() noexcept;

    private:
        /**
         * Vulkan API
         */
    public:
        vk::ImageAspectFlags _vkAspect;
        vk::ImageCreateFlags _vkCreate;
        vk::ImageLayout _vkLayout;
        vk::ImageUsageFlags _vkUsage;
    };
}
