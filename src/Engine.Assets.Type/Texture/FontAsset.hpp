#pragma once

#include <Engine.Assets.Type/Font/GlyphRanges.hpp>
#include <Engine.Filesystem/Url.hpp>

#include "../Asset.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::assets {
	class FontAsset final :
		public InheritMeta<FontAsset, Asset> {
	public:
		template <typename>
		friend class ::hg::engine::serialization::DataLayout;

		ACCESS_LAYOUT
		ACCESS_STRUCTURE

	public:
		constexpr static AssetTypeId typeId { "Font"_typeId };

	public:
		FontAsset();

		FontAsset(
			mref<AssetGuid> guid_,
			mref<StringView> name_,
			mref<AssetReferenceUrl> storageUrl_,
			mref<AssetUrl> vfsUrl_,
			mref<Vector<fs::Url>> sources_
		);

	private:
		Vector<fs::Url> _sources;

	public:
		[[nodiscard]] cref<Vector<fs::Url>> sources() const noexcept;
	};
}
