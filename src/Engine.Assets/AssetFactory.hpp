#pragma once

#include <Engine.Common/String.hpp>
#include <Engine.Common/Url.hpp>
#include <Engine.Common/Collection/Vector.hpp>
#include <Engine.Common/Math/Vector.hpp>
#include <Engine.GFX/TextureFormat.hpp>
#include <Engine.GFX/Texture/TextureType.hpp>

#include "Database/AssetDatabase.hpp"
#include "Types/Asset.hpp"

namespace ember::engine::assets {

    class AssetFactory {
    public:
        using value_type = AssetFactory;
        using reference_type = ref<value_type>;
        using const_reference_type = cref<value_type>;

    public:
        /**
         * Constructor
         *
         * @author Julius
         * @date 06.10.2021
         *
         * @param  database_ The database.
         */
        AssetFactory(ptr<AssetDatabase> database_) noexcept;

        /**
         * Destructor
         *
         * @author Julius
         * @date 06.10.2021
         */
        ~AssetFactory() noexcept;

    private:
        /**
         * Asset Database
         */
        ptr<AssetDatabase> _database;

    private:
        [[nodiscard]] Url resolveAsSource(cref<string> url_) const noexcept;

    public:
        /**
         * Creates a graphics material asset
         *
         * @author Julius
         * @date 06.10.2021
         *
         * @returns The new graphics material asset.
         */
        [[nodiscard]] ptr<Asset> createGfxMaterialAsset();

        /**
         * Creates a graphics material asset
         *
         * @author Julius
         * @date 06.10.2021
         *
         * @param  guid_ Unique identifier.
         *
         * @returns The new graphics material asset.
         */
        [[nodiscard]] ptr<Asset> createGfxMaterialAsset(cref<asset_guid> guid_);

        /**
         * Creates a graphics material asset
         *
         * @author Julius
         * @date 06.10.2021
         *
         * @param  guid_ Unique identifier.
         * @param  albedo_ The albedo.
         * @param  ao_ The ao.
         * @param  cavity_ The cavity.
         * @param  displacement_ The displacement.
         * @param  gloss_ The gloss.
         * @param  normal_ The normal.
         * @param  roughness_ The roughness.
         * @param  specular_ The specular.
         *
         * @returns The new graphics material asset.
         */
        [[nodiscard]] ptr<Asset> createGfxMaterialAsset(
            cref<asset_guid> guid_,
            cref<asset_guid> albedo_,
            cref<asset_guid> ao_,
            cref<asset_guid> cavity_,
            cref<asset_guid> displacement_,
            cref<asset_guid> gloss_,
            cref<asset_guid> normal_,
            cref<asset_guid> roughness_,
            cref<asset_guid> specular_
        );

    public:
        /**
         * Creates a image asset
         *
         * @author Julius
         * @date 06.10.2021
         *
         * @returns The new image asset.
         */
        [[nodiscard]] ptr<Asset> createImageAsset();

        /**
         * Creates image asset
         *
         * @author Julius
         * @date 06.10.2021
         *
         * @param  guid_ Unique identifier.
         *
         * @returns The new image asset.
         */
        [[nodiscard]] ptr<Asset> createImageAsset(cref<asset_guid> guid_);

        /**
         * Creates a image asset
         *
         * @author Julius
         * @date 06.10.2021
         *
         * @param  guid_ Unique identifier.
         * @param  url_ URL_ of the resource.
         *
         * @returns The new image asset.
         */
        [[nodiscard]] ptr<Asset> createImageAsset(cref<asset_guid> guid_, cref<string> url_);

    public:
        /**
         * Creates a texture asset
         *
         * @author Julius
         * @date 06.10.2021
         *
         * @returns The new texture asset.
         */
        [[nodiscard]] ptr<Asset> createTextureAsset();

        /**
         * Creates texture asset
         *
         * @author Julius
         * @date 06.10.2021
         *
         * @param  guid_ Unique identifier.
         *
         * @returns The new texture asset.
         */
        [[nodiscard]] ptr<Asset> createTextureAsset(cref<asset_guid> guid_);

        /**
         * Creates a texture asset
         *
         * @author Julius
         * @date 06.10.2021
         *
         * @param  guid_ Unique identifier.
         * @param  baseImage_ The base image.
         * @param  images_ The images.
         * @param  extent_ The extent.
         * @param  format_ Describes the format to use.
         * @param  mipLevel_ The mip level.
         * @param  type_ The type.
         *
         * @returns The new texture asset.
         */
        [[nodiscard]] ptr<Asset> createTextureAsset(
            cref<asset_guid> guid_,
            cref<asset_guid> baseImage_,
            mref<Vector<asset_guid>> images_,
            cref<math::uivec3> extent_,
            cref<gfx::TextureFormat> format_,
            cref<u32> mipLevel_,
            cref<gfx::TextureType> type_
        );

        // TODO:
    public:
        [[nodiscard]] ptr<Asset> createStaticGeometryAsset(cref<asset_guid> guid_);

        [[nodiscard]] ptr<Asset> createLandscapeGeometryAsset(cref<asset_guid> guid_);
    };

}
