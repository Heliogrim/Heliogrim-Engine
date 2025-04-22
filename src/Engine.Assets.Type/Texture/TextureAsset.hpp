#pragma once

#include <Engine.Common/Collection/Vector.hpp>
#include <Engine.Common/Math/Vector.hpp>
#include <Engine.GFX/TextureFormat.hpp>
#include <Engine.GFX/Texture/TextureType.hpp>
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
		constexpr static asset_type_id typeId { "Texture"_typeId };

	private:
	public:
		TextureAsset();

	protected:
		explicit TextureAsset(mref<AssetGuid> guid_);

	public:
		TextureAsset(
			mref<AssetGuid> guid_,
			cref<AssetGuid> baseImage_,
			mref<Vector<AssetGuid>> images_,
			cref<math::uivec3> extent_,
			cref<gfx::TextureFormat> format_,
			cref<u32> mipLevel_,
			cref<gfx::TextureType> type_
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
		gfx::TextureFormat _format;

	public:
		[[nodiscard]] gfx::TextureFormat getTextureFormat() const noexcept;

		void setTextureFormat(const gfx::TextureFormat format_);

	private:
		u32 _mipLevel;

	public:
		[[nodiscard]] u32 getMipLevelCount() const noexcept;

		void setMipLevelCount(const u32 levels_);

	private:
		gfx::TextureType _textureType;

	public:
		[[nodiscard]] gfx::TextureType getTextureType() const noexcept;

		void setTextureType(const gfx::TextureType type_);
	};
}

namespace hg::engine::assets {
	template <>
	struct MetaStreamLoaderAsset<::hg::engine::assets::TextureAsset> {
		using type = ::hg::engine::assets::TextureAsset;
		inline static constexpr bool value = true;
	};
}
