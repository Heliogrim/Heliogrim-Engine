#pragma once

#include <Engine.Assets.Type/Geometry/LandscapeGeometry.hpp>
#include <Engine.Assets.Type/Geometry/SkeletalGeometry.hpp>
#include <Engine.Assets.Type/Geometry/StaticGeometry.hpp>

#include "TypedAssetHandle.hpp"

namespace hg::engine::assets {
	class LandscapeGeometry;
	class StaticGeometry;
	class SkeletalGeometry;
}

namespace hg {
	class LandscapeGeometryAssetHandle final :
		public TypedAssetHandle<::hg::engine::assets::LandscapeGeometry> {
	public:
		using TypedAssetHandle::TypedAssetHandle;
	};

	class SkeletalGeometryAssetHandle final :
		public TypedAssetHandle<::hg::engine::assets::SkeletalGeometry> {
	public:
		using TypedAssetHandle::TypedAssetHandle;
	};

	class StaticGeometryAssetHandle final :
		public TypedAssetHandle<::hg::engine::assets::StaticGeometry> {
	public:
		using TypedAssetHandle::TypedAssetHandle;
	};

	/**/

	template <>
	template <>
	AssetTypeId TypedAssetHandle<engine::assets::LandscapeGeometry>::getStaticTypeId<engine::assets::LandscapeGeometry>() noexcept;

	template <>
	template <>
	AssetTypeId TypedAssetHandle<engine::assets::SkeletalGeometry>::getStaticTypeId<engine::assets::SkeletalGeometry>() noexcept;

	template <>
	template <>
	AssetTypeId TypedAssetHandle<engine::assets::StaticGeometry>::getStaticTypeId<engine::assets::StaticGeometry>() noexcept;
}
