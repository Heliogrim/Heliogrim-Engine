#pragma once

#include "TypedAssetHandle.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Assets.Type;
#endif

namespace hg::engine::assets {
	class GraphicsMaterial;
	class GraphicsMaterialPrototype;
	class PhysicsMaterial;
	class AudioMaterial;
}

namespace hg {
	class GraphicsMaterialAssetHandle final :
		public TypedAssetHandle<::hg::engine::assets::GraphicsMaterial> {
	public:
		using TypedAssetHandle::TypedAssetHandle;
	};

	class GraphicsMaterialPrototypeAssetHandle final :
		public TypedAssetHandle<::hg::engine::assets::GraphicsMaterialPrototype> {
	public:
		using TypedAssetHandle::TypedAssetHandle;
	};

	class PhysicsMaterialAssetHandle final :
		public TypedAssetHandle<::hg::engine::assets::PhysicsMaterial> {
	public:
		using TypedAssetHandle::TypedAssetHandle;
	};

	class AudioMaterialAssetHandle final :
		public TypedAssetHandle<::hg::engine::assets::AudioMaterial> {
	public:
		using TypedAssetHandle::TypedAssetHandle;
	};

	/**/

	template <>
	template <>
	AssetTypeId TypedAssetHandle<engine::assets::GraphicsMaterial>::getStaticTypeId<engine::assets::GraphicsMaterial>() noexcept;

	template <>
	template <>
	AssetTypeId TypedAssetHandle<engine::assets::GraphicsMaterialPrototype>::getStaticTypeId<engine::assets::GraphicsMaterialPrototype>() noexcept;

	template <>
	template <>
	AssetTypeId TypedAssetHandle<engine::assets::PhysicsMaterial>::getStaticTypeId<engine::assets::PhysicsMaterial>() noexcept;

	template <>
	template <>
	AssetTypeId TypedAssetHandle<engine::assets::AudioMaterial>::getStaticTypeId<engine::assets::AudioMaterial>() noexcept;
}
