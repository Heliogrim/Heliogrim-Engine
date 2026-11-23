#pragma once

#include <Engine.Assets.Type/Texture/FontAsset.hpp>
#include <Engine.Assets.Type/Texture/ImageAsset.hpp>
#include <Engine.Assets.Type/Texture/TextureAsset.hpp>

#include "TypedAssetHandle.hpp"

namespace hg::engine::assets {
	class FontAsset;
	class ImageAsset;
	class TextureAsset;
}

namespace hg {
	class FontAssetHandle final :
		public TypedAssetHandle<::hg::engine::assets::FontAsset> {
	public:
		using TypedAssetHandle::TypedAssetHandle;
	};

	class ImageAssetHandle final :
		public TypedAssetHandle<::hg::engine::assets::ImageAsset> {
	public:
		using TypedAssetHandle::TypedAssetHandle;
	};

	class TextureAssetHandle final :
		public TypedAssetHandle<::hg::engine::assets::TextureAsset> {
	public:
		using TypedAssetHandle::TypedAssetHandle;
	};

	/**/

	template <>
	template <>
	AssetTypeId TypedAssetHandle<engine::assets::FontAsset>::getStaticTypeId<engine::assets::FontAsset>() noexcept;

	template <>
	template <>
	AssetTypeId TypedAssetHandle<engine::assets::ImageAsset>::getStaticTypeId<engine::assets::ImageAsset>() noexcept;

	template <>
	template <>
	AssetTypeId TypedAssetHandle<engine::assets::TextureAsset>::getStaticTypeId<engine::assets::TextureAsset>() noexcept;
}
