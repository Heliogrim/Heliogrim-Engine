#pragma once

#include <Engine.Common/Types.hpp>
#include <Engine.Common/Collection/Vector.hpp>
#include "../Device/Device.hpp"

namespace hg::engine::gfx {
    /**
     * Forward Declaration Framebuffer.
     *
     * @author Julius
     * @date 11.12.2020
     */
    class Framebuffer;
}

namespace hg::engine::gfx::pipeline {
    class LORenderPass {
    public:
        /**
         * Constructor
         *
         * @author Julius
         * @date 03.12.2020
         *
         * @param  device_ The device.
         */
        LORenderPass(sptr<Device> device_) noexcept;

        /**
         * Destructor
         *
         * @author Julius
         * @date 03.12.2020
         */
        ~LORenderPass();

        /**
         * Setups this
         *
         * @author Julius
         * @date 03.12.2020
         */
        void setup();

        /**
         * Setups this
         *
         * @author Julius
         * @date 03.12.2020
         *
         * @param  framebuffer_ The framebuffer.
         */
        void setup(const Framebuffer& framebuffer_);

        /**
         * Destroys this 
         *
         * @author Julius
         * @date 03.12.2020
         */
        void destroy();

    private:
        /**
         * Attachments
         */
        Vector<vk::AttachmentDescription> _attachments;
        Vector<vk::AttachmentReference> _references;

    public:
        /**
         * Gets the attachments
         *
         * @author Julius
         * @date 03.12.2020
         *
         * @returns A list of.
         */
        [[nodiscard]] const Vector<vk::AttachmentDescription>& attachments() const noexcept;

        /**
         * Attachments the given index
         *
         * @author Julius
         * @date 03.12.2020
         *
         * @param  idx_ Zero-based index of the.
         *
         * @returns A reference to a vk::AttachmentDescription.
         */
        [[nodiscard]] vk::AttachmentDescription& attachment(const u32 idx_);

        /**
         * Sets the attachment with the given idx
         *
         * @author Julius
         * @date 03.12.2020
         *
         * @param  idx_ Zero-based index of the.
         * @param  attachment_ The attachment.
         */
        void set(const u32 idx_, const vk::AttachmentDescription& attachment_);

    private:
        /**
         * Dependencies
         */
        Vector<vk::SubpassDependency> _dependencies;

    public:
        /**
         * Gets the dependencies
         *
         * @author Julius
         * @date 03.12.2020
         *
         * @returns A reference to a collection::vector&lt;vk::SubpassDependency&gt;
         */
        [[nodiscard]] Vector<vk::SubpassDependency>& dependencies() noexcept;

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
         * @date 03.12.2020
         *
         * @returns A cref&lt;sptr&lt;Device&gt;&gt;
         */
        [[nodiscard]] cref<sptr<Device>> device() const noexcept;

    private:
        /**
         * Subpass
         */
        vk::SubpassDescription _subpass;

    public:
        /**
         * Gets the subpass
         *
         * @author Julius
         * @date 03.12.2020
         *
         * @returns A reference to a vk::SubpassDescription.
         */
        [[nodiscard]] vk::SubpassDescription& subpass() noexcept;

    private:
        /**
         * Vulkan API
         */
        vk::RenderPass _vkRenderPass;

    public:
        /**
         * Vk render pass
         *
         * @author Julius
         * @date 14.12.2020
         *
         * @returns A reference to a const vk::RenderPass.
         */
        [[nodiscard]] const vk::RenderPass& vkRenderPass() const noexcept;

        /**
         * Vk render pass
         *
         * @author Julius
         * @date 03.12.2020
         *
         * @returns A reference to the vk::RenderPass.
         */
        [[nodiscard]] vk::RenderPass& vkRenderPass() noexcept;

    private:
        /**
         * Vulkan API
         */
        vk::RenderPassBeginInfo _vkBeginInfo;
        Vector<vk::ClearValue> _clearValues;

    public:
        /**
         * Vk begin information
         *
         * @author Julius
         * @date 13.02.2021
         *
         * @returns A cref&lt;vk::RenderPassBeginInfo&gt;
         */
        [[nodiscard]] cref<vk::RenderPassBeginInfo> vkBeginInfo() const noexcept;

        //private:
    public:
        /**
         * Vulkan API
         */
        Vector<u32> _viewMasks;
        Vector<u32> _correlationMasks;
    };
}
