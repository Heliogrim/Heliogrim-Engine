#pragma once

#include "../Texture/Texture.hpp"

namespace ember::engine::gfx {
    class FramebufferAttachment final {
    public:
        /**
         * Default constructor
         *
         * @author Julius
         * @date 21.11.2020
         */
        FramebufferAttachment() = default;

        /**
         * Copy Constructor
         *
         * @author Julius
         * @date 12.12.2020
         */
        FramebufferAttachment(const FramebufferAttachment&) = default;

        /**
         * Move Constructor
         *
         * @author Julius
         * @date 12.12.2020
         *
         * @param [in,out] attachment_ The attachment.
         */
        FramebufferAttachment(FramebufferAttachment&& attachment_) noexcept = default;

        /**
         * Constructor
         *
         * @author Julius
         * @date 16.12.2020
         *
         * @param [in] texture_ The texture.
         */
        FramebufferAttachment(Texture&& texture_) noexcept;

        /**
         * Constructor
         *
         * @author Julius
         * @date 31.01.2021
         *
         * @param  texture_ The texture.
         */
        FramebufferAttachment(cref<sptr<Texture>> texture_) noexcept;

        /**
         * Destructor
         *
         * @author Julius
         * @date 21.11.2020
         */
        ~FramebufferAttachment() noexcept = default;

        /**
         * Copy Assignment operator
         *
         * @author Julius
         * @date 12.12.2020
         *
         * @returns A shallow copy of this.
         */
        FramebufferAttachment& operator=(cref<FramebufferAttachment> other_) = default;

        /**
         * Move assignment operator
         *
         * @author Julius
         * @date 12.12.2020
         *
         * @param [in,out] other_ The attachment.
         *
         * @returns A shallow copy of this.
         */
        FramebufferAttachment& operator=(FramebufferAttachment&& other_) noexcept = default;

        /**
         * Member dereference operator
         *
         * @author Julius
         * @date 31.01.2021
         *
         * @returns The dereferenced object.
         */
        ptr<const Texture> operator->() const noexcept;

        /**
         * Member dereference operator
         *
         * @author Julius
         * @date 31.01.2021
         *
         * @returns The dereferenced object.
         */
        ptr<Texture> operator->() noexcept;

    private:
        /**
         * Texture
         */
        sptr<Texture> _texture;

    public:
        /**
         * Gets the unwrapped
         *
         * @author Julius
         * @date 31.01.2021
         *
         * @returns A cref&lt;sptr&lt;Texture&gt;&gt;
         */
        [[nodiscard]] cref<sptr<Texture>> unwrapped() const noexcept;

        /**
         * Gets the unwrapped
         *
         * @author Julius
         * @date 31.01.2021
         *
         * @returns A ref&lt;sptr&lt;Texture&gt;&gt;
         */
        [[nodiscard]] ref<sptr<Texture>> unwrapped() noexcept;
    };
}
