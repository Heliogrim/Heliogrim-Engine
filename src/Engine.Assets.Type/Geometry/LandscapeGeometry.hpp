#pragma once

#include <Engine.Common/Collection/Vector.hpp>
#include <Engine.Filesystem/Url.hpp>

#include "Geometry.hpp"

namespace hg::engine::assets {
	class LandscapeGeometry final :
		public InheritMeta<LandscapeGeometry, Geometry> {
	public:
		template <typename>
		friend class ::hg::engine::serialization::DataLayout;

	public:
		constexpr static AssetTypeId typeId { "LandscapeGeometry"_typeId };

	public:
		LandscapeGeometry(
			mref<AssetGuid> guid_,
			mref<StringView> name_,
			mref<AssetReferenceUrl> storageUrl_,
			mref<AssetUrl> vfsUrl_,
			mref<Vector<fs::Url>> sources_
		);

	private:
		Vector<fs::Url> _sources;
	};
}
