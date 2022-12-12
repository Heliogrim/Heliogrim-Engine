#pragma once

#include "StreamableRenderableAsset.hpp"

namespace ember {
    class TextureAsset;

    class GfxMaterialAsset :
        public StreamableRenderableAsset {
    public:
        using base_type = StreamableRenderableAsset;

        using value_type = GfxMaterialAsset;
        using reference_type = ref<value_type>;
        using const_reference_type = cref<value_type>;

    public:
        /**
         * Constructor
         *
         * @author Julius
         * @date 06.10.2021
         *
         * @param  guid_ Unique identifier.
         */
        GfxMaterialAsset(cref<asset_guid> guid_) noexcept;

    protected:
        /**
         * Constructor
         *
         * @author Julius
         * @date 06.10.2021
         *
         * @param  guid_ Unique identifier.
         * @param  albedo_ The albedo.
         * @param  ao_ The ambient occlusion.
         * @param  cavity_ The cavity.
         * @param  displacement_ The displacement.
         * @param  gloss_ The gloss.
         * @param  normal_ The normal.
         * @param  roughness_ The roughness.
         * @param  metalness_ The metalness.
         * @param  specular_ The specular.
         * @param  alpha_ The alpha.
         */
        GfxMaterialAsset(
            cref<asset_guid> guid_,
            cref<asset_guid> albedo_,
            cref<asset_guid> ao_,
            cref<asset_guid> cavity_,
            cref<asset_guid> displacement_,
            cref<asset_guid> gloss_,
            cref<asset_guid> normal_,
            cref<asset_guid> roughness_,
            cref<asset_guid> metalness_,
            cref<asset_guid> specular_,
            cref<asset_guid> alpha_
        ) noexcept;

    public:
        /**
         * Destructor
         *
         * @author Julius
         * @date 06.10.2021
         */
        ~GfxMaterialAsset() noexcept;

    public:
        /**
         * Query if this is valid type
         *
         * @author Julius
         * @date 06.10.2021
         *
         * @returns True if valid type, false if not.
         */
        [[nodiscard]] bool isValidType() const noexcept;

    public:
        [[nodiscard]] const ptr<const TextureAsset> albedo() const noexcept;

        [[nodiscard]] cref<asset_guid> albedoGuid() const noexcept;

        [[nodiscard]] const ptr<const TextureAsset> ao() const noexcept;

        [[nodiscard]] cref<asset_guid> aoGuid() const noexcept;

        [[nodiscard]] const ptr<const TextureAsset> cavity() const noexcept;

        [[nodiscard]] cref<asset_guid> cavityGuid() const noexcept;

        [[nodiscard]] const ptr<const TextureAsset> displacement() const noexcept;

        [[nodiscard]] cref<asset_guid> displacementGuid() const noexcept;

        [[nodiscard]] const ptr<const TextureAsset> gloss() const noexcept;

        [[nodiscard]] cref<asset_guid> glossGuid() const noexcept;

        [[nodiscard]] const ptr<const TextureAsset> normal() const noexcept;

        [[nodiscard]] cref<asset_guid> normalGuid() const noexcept;

        [[nodiscard]] const ptr<const TextureAsset> roughness() const noexcept;

        [[nodiscard]] cref<asset_guid> roughnessGuid() const noexcept;

        [[nodiscard]] const ptr<const TextureAsset> metalness() const noexcept;

        [[nodiscard]] cref<asset_guid> metalnessGuid() const noexcept;

        [[nodiscard]] const ptr<const TextureAsset> specular() const noexcept;

        [[nodiscard]] cref<asset_guid> specularGuid() const noexcept;

        [[nodiscard]] const ptr<const TextureAsset> alpha() const noexcept;

        [[nodiscard]] cref<asset_guid> alphaGuid() const noexcept;
    };
}
