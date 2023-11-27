#pragma once

#include <Engine.Common/String.hpp>
#include <Engine.Common/Collection/Vector.hpp>
#include <Engine.Common/Math/Vector.hpp>
#include <Engine.GFX/TextureFormat.hpp>
#include <Engine.GFX/Texture/TextureType.hpp>
#include <Engine.Assets.System/__fwd.hpp>
#include <Engine.Filesystem/__fwd.hpp>
#include <Engine.Filesystem/Url.hpp>

#include "Types/Asset.hpp"

namespace hg::engine::assets {
    class AccelEffect;
    class Font;
    class GfxMaterial;
    class GfxMaterialPrototype;
    class Image;
    class LandscapeGeometry;
    class Level;
    class PfxMaterial;
    class SfxMaterial;
    class SkeletalGeometry;
    class Sound;
    class StaticGeometry;
    class TextureAsset;
}

namespace hg::engine::assets {
    /**/
    extern void storeDefaultNameAndUrl(non_owning_rptr<Asset> asset_, string source_);

    /**/

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
         */
        AssetFactory(
            const non_owning_rptr<IAssetRegistry> registry_,
            const non_owning_rptr<system::InMemAssetRepository> repository_
        ) noexcept;

        /**
         * Destructor
         *
         * @author Julius
         * @date 06.10.2021
         */
        ~AssetFactory() noexcept;

    private:
        non_owning_rptr<IAssetRegistry> _registry;
        non_owning_rptr<system::InMemAssetRepository> _repository;

    private:
        [[nodiscard]] fs::Url resolveAsSource(cref<string> url_) const noexcept;

    public:
        /**
         * Will generate serialization/deserialization hooks for asset types
         */
        void prepare();

    public:
        [[nodiscard]] ptr<Font> createFontAsset(cref<asset_guid> guid_) const;

        [[nodiscard]] ptr<Font> createFontAsset(cref<asset_guid> guid_, cref<string> url_) const;

        [[nodiscard]] ptr<AccelEffect> createAccelEffectAsset(cref<asset_guid> guid_) const;

        [[nodiscard]] ptr<GfxMaterial> createGfxMaterialAsset(
            mref<asset_guid> prototypeGuid_
        ) const;

        [[nodiscard]] ptr<GfxMaterial> createGfxMaterialAsset(
            cref<asset_guid> guid_,
            mref<asset_guid> prototypeGuid_
        ) const;

        [[nodiscard]] ptr<GfxMaterialPrototype> createGfxMaterialPrototypeAsset(cref<asset_guid> guid_) const;

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
        [[nodiscard]] ptr<TextureAsset> createTextureAsset() const;

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
        [[nodiscard]] ptr<TextureAsset> createTextureAsset(cref<asset_guid> guid_) const;

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
        [[nodiscard]] ptr<TextureAsset> createTextureAsset(
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
