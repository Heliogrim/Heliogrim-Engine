#pragma once

#include <Engine.Assets.Type/Material/GfxMaterial.hpp>
#include <Engine.Assets.Type/Material/GfxMaterialPrototype.hpp>
#include <Engine.Assets.Type/Material/PfxMaterial.hpp>
#include <Engine.Assets.Type/Material/SfxMaterial.hpp>

#include "TypedAssetHandle.hpp"

namespace hg::engine::assets {
	class GfxMaterial;
	class GfxMaterialPrototype;
	class PfxMaterial;
	class SfxMaterial;
}

namespace hg {
	class GfxMaterialAssetHandle final :
		public TypedAssetHandle<::hg::engine::assets::GfxMaterial> {
	public:
		using TypedAssetHandle::TypedAssetHandle;
	};

	class GfxMaterialPrototypeAssetHandle final :
		public TypedAssetHandle<::hg::engine::assets::GfxMaterialPrototype> {
	public:
		using TypedAssetHandle::TypedAssetHandle;
	};

	class PfxMaterialAssetHandle final :
		public TypedAssetHandle<::hg::engine::assets::PfxMaterial> {
	public:
		using TypedAssetHandle::TypedAssetHandle;
	};

	class SfxMaterialAssetHandle final :
		public TypedAssetHandle<::hg::engine::assets::SfxMaterial> {
	public:
		using TypedAssetHandle::TypedAssetHandle;
	};

	/**/

	template <>
	template <>
	AssetTypeId TypedAssetHandle<engine::assets::GfxMaterial>::getStaticTypeId<engine::assets::GfxMaterial>() noexcept;

	template <>
	template <>
	AssetTypeId TypedAssetHandle<engine::assets::GfxMaterialPrototype>::getStaticTypeId<engine::assets::GfxMaterialPrototype>() noexcept;

	template <>
	template <>
	AssetTypeId TypedAssetHandle<engine::assets::PfxMaterial>::getStaticTypeId<engine::assets::PfxMaterial>() noexcept;

	template <>
	template <>
	AssetTypeId TypedAssetHandle<engine::assets::SfxMaterial>::getStaticTypeId<engine::assets::SfxMaterial>() noexcept;
}
