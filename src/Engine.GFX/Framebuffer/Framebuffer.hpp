#pragma once

#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Collection/Vector.hpp>
#include <Engine.Common/Math/Vector.hpp>
#include <Engine.Accel.Pass/__fwd.hpp>

#include "../Device/Device.hpp"
#include "Engine.GFX/Texture/TextureLikeObject.hpp"

namespace hg::engine::gfx {
    class Framebuffer {
    public:
        /**
         * Constructor
         *
         * @author Julius
         * @date 21.11.2020
         *
         * @param  device_ The device.
         */
        Framebuffer(cref<sptr<Device>> device_);

        /**
         * Copy constructor
         *
         * @author Julius
         * @date 12.12.2020
         */
        Framebuffer(cref<Framebuffer>) = delete;

        /**
         * Move Constructor
         *
         * @author Julius
         * @date 01.02.2021
         *
         * @param  other_ The other.
         */
        Framebuffer(mref<Framebuffer> other_) noexcept;

        /**
         * Destructor
         *
         * @author Julius
         * @date 21.11.2020
         */
        ~Framebuffer() noexcept;

        /**
         * Destroys this 
         *
         * @author Julius
         * @date 21.11.2020
         */
        void destroy();

        /**
         * Setups this 
         *
         * @author Julius
         * @date 21.11.2020
         */
        void setup();

    private:
        Vector<smr<TextureLikeObject>> _attachments;

    public:
        void addAttachment(mref<smr<TextureLikeObject>> textureLikeObject_);

        [[nodiscard]] cref<Vector<smr<TextureLikeObject>>> attachments() const noexcept;

    private:
        /**
         * Extent
         */
        math::uivec3 _extent;

    public:
        /**
         * Gets the extent
         *
         * @author Julius
         * @date 31.01.2021
         *
         * @returns A cref&lt;math::uivec3&gt;
         */
        [[nodiscard]] cref<math::uivec3> extent() const noexcept;

        /**
         * Gets the width
         *
         * @author Julius
         * @date 21.11.2020
         *
         * @returns An u32.
         */
        [[nodiscard]] u32 width() const noexcept;

        /**
         * Gets the height
         *
         * @author Julius
         * @date 21.11.2020
         *
         * @returns An u32.
         */
        [[nodiscard]] u32 height() const noexcept;

        /**
         * Sets an extent
         *
         * @author Julius
         * @date 03.12.2020
         *
         * @param  extent_ The extent.
         */
        void setExtent(cref<math::uivec3> extent_) noexcept;

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
         * @date 21.11.2020
         *
         * @returns A const.
         */
        [[nodiscard]] cref<sptr<Device>> device() const noexcept;

    private:
        /**
         * Render Pass
         */
        smr<const accel::AccelerationPass> _renderPass;

    public:
        [[nodiscard]] cref<smr<const accel::AccelerationPass>> renderPass() const noexcept;

        void setRenderPass(mref<smr<const accel::AccelerationPass>> renderPass_);

    private:
        /**
         * Vulkan API
         */
        vk::Framebuffer _vkFramebuffer;

    public:
        /**
         * Gets the framebuffer
         *
         * @author Julius
         * @date 21.11.2020
         *
         * @returns A reference to a const vk::Framebuffer.
         */
        [[nodiscard]] const vk::Framebuffer& vkFramebuffer() const noexcept;

        /**
         * Cast that converts this to a vk::Framebuffer
         *
         * @author Julius
         * @date 21.11.2020
         *
         * @returns The result of the operation.
         */
        operator vk::Framebuffer() const noexcept;
    };
}
