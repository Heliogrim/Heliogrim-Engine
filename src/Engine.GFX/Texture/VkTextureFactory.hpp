#pragma once
#include "TextureFactory.hpp"

namespace hg::engine::gfx {
    class VkTextureFactory final :
        public TextureFactory {
    public:
        /**
         * Gets the make
         *
         * @author Julius
         * @date 07.12.2020
         *
         * @param  device_ The device.
         *
         * @returns A ptr&lt;TextureFactory&gt;
         */
        static ptr<TextureFactory> make(cref<sptr<Device>> device_);

    private:
        /**
         * Constructor
         *
         * @author Julius
         * @date 07.12.2020
         *
         * @param  device_ The device.
         */
        VkTextureFactory(cref<sptr<Device>> device_);

        /**
         * Destructor
         *
         * @author Julius
         * @date 07.12.2020
         */
        ~VkTextureFactory() noexcept override;

    public:
        /**
         * Builds the given payload
         *
         * @author Julius
         * @date 07.12.2020
         *
         * @param  payload_ The payload.
         *
         * @returns A Texture.
         */
        [[nodiscard]] Texture build(
            const TextureBuildPayload& payload_
        ) const override;

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
        Texture& buildView(
            Texture& texture_,
            TextureViewBuildOptions options_ = {}
        ) const override;

        ref<TextureView> buildView(
            ref<TextureView> texture_,
            TextureViewBuildOptions options_
        ) const override;

        ref<SparseTexture> buildView(
            ref<SparseTexture> texture_,
            TextureViewBuildOptions options_
        ) const override;

        ref<SparseTextureView> buildView(
            ref<SparseTextureView> texture_,
            TextureViewBuildOptions options_
        ) const override;

        /**
         * Builds a virtual
         *
         * @author Julius
         * @date 07.12.2020
         *
         * @param  payload_ The payload.
         *
         * @returns A Texture.
         */
        [[nodiscard]] ptr<SparseTexture> buildVirtual(
            const SparseTextureBuildPayload& payload_
        ) const override;
    };
}
