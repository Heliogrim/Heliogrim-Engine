#pragma once

#include <Engine.Common/Collection/Vector.hpp>
#include <Engine.Filesystem/Url.hpp>

#include "Geometry.hpp"

namespace hg::engine::assets {
	class LandscapeGeometry :
		public InheritMeta<LandscapeGeometry, Geometry> {
	public:
		template <typename>
		friend class ::hg::engine::serialization::DataLayout;

	public:
		constexpr static asset_type_id typeId { "LandscapeGeometry"_typeId };

	protected:
		LandscapeGeometry(mref<AssetGuid> guid_);

	public:
		LandscapeGeometry(mref<AssetGuid> guid_, mref<Vector<fs::Url>> sources_);

	private:
		Vector<fs::Url> _sources;
	};
}
