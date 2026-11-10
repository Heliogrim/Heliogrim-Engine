#pragma once

#include <Engine.Common/Collection/Vector.hpp>
#include <Engine.Common/Math/Vector.hpp>
#include <Engine.Graphics/TextureFormat.hpp>
#include <Engine.Graphics/Texture/TextureType.hpp>
#include <Engine.Serialization/Access/__fwd.hpp>

#include "../AssetConcept.hpp"

namespace hg::engine::assets {
	class TextureAsset :
		public InheritMeta<TextureAsset, Asset> {
	public:
		template <typename>
		friend class ::hg::engine::serialization::DataLayout;

		ACCESS_LAYOUT
		ACCESS_STRUCTURE

	public:
		constexpr static AssetTypeId typeId { "Texture"_typeId };

	public:
		TextureAsset();

		TextureAsset(
			mref<AssetGuid> guid_,
			mref<StringView> name_,
			mref<AssetReferenceUrl> storageUrl_,
			mref<AssetUrl> vfsUrl_,
			cref<AssetGuid> baseImage_,
			mref<Vector<AssetGuid>> images_,
			cref<math::uivec3> extent_,
			cref<graphics::TextureFormat> format_,
			cref<u32> mipLevel_,
			cref<graphics::TextureType> type_
		);

	private:
		AssetGuid _baseImage;
		Vector<AssetGuid> _images;

	public:
		[[nodiscard]] cref<AssetGuid> baseImage() const noexcept;

		void setBaseImage(const AssetGuid imageGuid_);

		void addSourceImage(const AssetGuid imageGuid_);

		[[nodiscard]] bool hasSourceImage(const AssetGuid imageGuid_);

		void removeSourceImage(const AssetGuid imageGuid_);

	private:
		math::uivec3 _extent;

	public:
		[[nodiscard]] cref<math::uivec3> getExtent() const noexcept;

		void setExtent(cref<math::uivec3> extent_);

	private:
		graphics::TextureFormat _format;

	public:
		[[nodiscard]] graphics::TextureFormat getTextureFormat() const noexcept;

		void setTextureFormat(const graphics::TextureFormat format_);

	private:
		u32 _mipLevel;

	public:
		[[nodiscard]] u32 getMipLevelCount() const noexcept;

		void setMipLevelCount(const u32 levels_);

	private:
		graphics::TextureType _textureType;

	public:
		[[nodiscard]] graphics::TextureType getTextureType() const noexcept;

		void setTextureType(const graphics::TextureType type_);
	};
}

namespace hg::engine::assets {
	template <>
	struct MetaStreamLoaderAsset<::hg::engine::assets::TextureAsset> {
		using type = ::hg::engine::assets::TextureAsset;
		inline static constexpr bool value = true;
	};
}
