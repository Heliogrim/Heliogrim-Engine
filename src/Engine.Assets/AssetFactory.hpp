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
    class Font;
    class GfxMaterial;
    class Image;
    class LandscapeGeometry;
    class Level;
    class PfxMaterial;
    class SfxMaterial;
    class SkeletalGeometry;
    class Sound;
    class StaticGeometry;
    class Texture;
}

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
         * Will generate serialization/deserialization hooks for asset types
         */
        void prepare();

    public:
        [[nodiscard]] ptr<Font> createFontAsset(cref<asset_guid> guid_) const;

        [[nodiscard]] ptr<Font> createFontAsset(cref<asset_guid> guid_, cref<string> url_) const;

        /**
         * Creates a graphics material asset
         *
         * @author Julius
         * @date 06.10.2021
         *
         * @returns The new graphics material asset.
         */
        [[nodiscard]] ptr<GfxMaterial> createGfxMaterialAsset() const;

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
        [[nodiscard]] ptr<GfxMaterial> createGfxMaterialAsset(cref<asset_guid> guid_) const;

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
         * @param  metalness_ The metalness.
         * @param  specular_ The specular.
         * @param  alpha_ The alpha.
         *
         * @returns The new graphics material asset.
         */
        [[nodiscard]] ptr<GfxMaterial> createGfxMaterialAsset(
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
        ) const;

    public:
        /**
         * Creates a image asset
         *
         * @author Julius
         * @date 06.10.2021
         *
         * @returns The new image asset.
         */
        [[nodiscard]] ptr<Image> createImageAsset() const;

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
        [[nodiscard]] ptr<Image> createImageAsset(cref<asset_guid> guid_) const;

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
        [[nodiscard]] ptr<Image> createImageAsset(cref<asset_guid> guid_, cref<string> url_) const;

    public:
        /**
         * Creates a texture asset
         *
         * @author Julius
         * @date 06.10.2021
         *
         * @returns The new texture asset.
         */
        [[nodiscard]] ptr<Texture> createTextureAsset() const;

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
        [[nodiscard]] ptr<Texture> createTextureAsset(cref<asset_guid> guid_) const;

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
        [[nodiscard]] ptr<Texture> createTextureAsset(
            cref<asset_guid> guid_,
            cref<asset_guid> baseImage_,
            mref<Vector<asset_guid>> images_,
            cref<math::uivec3> extent_,
            cref<gfx::TextureFormat> format_,
            cref<u32> mipLevel_,
            cref<gfx::TextureType> type_
        ) const;

        // TODO:
    public:
        [[nodiscard]] ptr<StaticGeometry> createStaticGeometryAsset(cref<asset_guid> guid_) const;

        [[nodiscard]] ptr<StaticGeometry> createStaticGeometryAsset(
            cref<asset_guid> guid_,
            cref<string> url_,
            cref<u64> vertexCount_,
            cref<u64> indexCount_
        ) const;

        [[nodiscard]] ptr<LandscapeGeometry> createLandscapeGeometryAsset(cref<asset_guid> guid_) const;
    };
}
