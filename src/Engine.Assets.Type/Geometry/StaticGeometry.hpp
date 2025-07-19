#pragma once

#include <Engine.Common/Collection/AutoArray.hpp>
#include <Engine.Common/Collection/Vector.hpp>
#include <Engine.Common/Math/Bounding.hpp>
#include <Engine.Filesystem/Url.hpp>
#include <Engine.Serialization/Access/__fwd.hpp>
#include <Heliogrim/TypedAssetGuid.hpp>

#include "Geometry.hpp"
#include "../Material/GfxMaterial.hpp"

namespace hg::engine::assets {
	class StaticGeometry final :
		public InheritMeta<StaticGeometry, Geometry> {
	public:
		template <typename>
		friend class ::hg::engine::serialization::DataLayout;

		ACCESS_LAYOUT
		ACCESS_STRUCTURE

	public:
		constexpr static AssetTypeId typeId { "StaticGeometry"_typeId };

	public:
		StaticGeometry();

		StaticGeometry(
			mref<AssetGuid> guid_,
			mref<StringView> name_,
			mref<AssetReferenceUrl> storageUrl_,
			mref<AssetUrl> vfsUrl_,
			mref<Vector<fs::Url>> sources_,
			mref<AutoArray<AssetGuid>> meshMaterials_
		);

	private:
		Vector<fs::Url> _sources;

	public:
		[[nodiscard]] cref<Vector<fs::Url>> sources() const noexcept;

	private:
		// TODO: Check whether we can support `AutoArray<TypedAssetGuid<GfxMaterial>>`
		// TODO:	As of now this will break in serialization due to ordering
		AutoArray<AssetGuid> _meshMaterials;

	public:
		[[nodiscard]] u16 getMeshMaterialCount() const noexcept;

		[[nodiscard]] ref<const AutoArray<AssetGuid>> getMeshMaterials() const noexcept;

		void setMeshMaterial(u16 index_, mref<TypedAssetGuid<GfxMaterial>> materialGuid_);

		/* TODO: Rework !?! */
		//private:
	public:
		u64 _vertexCount;
		u64 _indexCount;

		u64 _clusterCount;
		u64 _clusterDepth;
	};
}
