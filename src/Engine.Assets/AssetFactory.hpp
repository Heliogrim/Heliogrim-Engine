#pragma once

#include <Engine.Assets.System/__fwd.hpp>
#include <Engine.Common/String.hpp>
#include <Engine.Common/Collection/Vector.hpp>
#include <Engine.Common/Managed/Rc.hpp>
#include <Engine.Common/Math/Vector.hpp>
#include <Engine.Filesystem/Url.hpp>
#include <Engine.Filesystem/__fwd.hpp>
#include <Engine.GFX/TextureFormat.hpp>
#include <Engine.GFX/Texture/TextureType.hpp>

#include "AssetGuid.hpp"

namespace hg::engine::assets {
	class Asset;
	class AccelEffect;
	class Font;
	class GfxMaterial;
	class GfxMaterialPrototype;
	class Image;
	class LandscapeGeometry;
	class LevelAsset;
	class PfxMaterial;
	class SfxMaterial;
	class SkeletalGeometry;
	class Sound;
	class StaticGeometry;
	class TextureAsset;
}

namespace hg::engine::assets {
	/**/
	extern void storeDefaultNameAndUrl(ref<Asset> asset_, string source_);

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
		Arci<Font> createFontAsset(mref<AssetGuid> guid_) const;

		Arci<Font> createFontAsset(mref<AssetGuid> guid_, cref<string> url_) const;

		Arci<AccelEffect> createAccelEffectAsset(mref<AssetGuid> guid_) const;

		Arci<GfxMaterial> createGfxMaterialAsset(mref<AssetGuid> prototypeGuid_) const;

		Arci<GfxMaterial> createGfxMaterialAsset(
			mref<AssetGuid> guid_,
			mref<AssetGuid> prototypeGuid_
		) const;

		Arci<GfxMaterialPrototype> createGfxMaterialPrototypeAsset(mref<AssetGuid> guid_) const;

		Arci<LevelAsset> createLevelAsset(mref<AssetGuid> guid_) const;

	public:
		/**
		 * Creates a image asset
		 *
		 * @author Julius
		 * @date 06.10.2021
		 *
		 * @returns The new image asset.
		 */
		Arci<Image> createImageAsset() const;

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
		Arci<Image> createImageAsset(mref<AssetGuid> guid_) const;

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
		Arci<Image> createImageAsset(mref<AssetGuid> guid_, cref<string> url_) const;

	public:
		/**
		 * Creates a texture asset
		 *
		 * @author Julius
		 * @date 06.10.2021
		 *
		 * @returns The new texture asset.
		 */
		Arci<TextureAsset> createTextureAsset() const;

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
		Arci<TextureAsset> createTextureAsset(mref<AssetGuid> guid_) const;

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
		Arci<TextureAsset> createTextureAsset(
			mref<AssetGuid> guid_,
			cref<AssetGuid> baseImage_,
			mref<Vector<AssetGuid>> images_,
			cref<math::uivec3> extent_,
			cref<gfx::TextureFormat> format_,
			cref<u32> mipLevel_,
			cref<gfx::TextureType> type_
		) const;

		// TODO:
	public:
		Arci<StaticGeometry> createStaticGeometryAsset(mref<AssetGuid> guid_) const;

		Arci<StaticGeometry> createStaticGeometryAsset(
			mref<AssetGuid> guid_,
			cref<string> url_,
			cref<u64> vertexCount_,
			cref<u64> indexCount_
		) const;

		Arci<LandscapeGeometry> createLandscapeGeometryAsset(mref<AssetGuid> guid_) const;
	};
}
