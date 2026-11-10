#pragma once

#include <Engine.Assets.Type/Material/GraphicsMaterial.hpp>
#include <Engine.Assets.Type/Material/GraphicsMaterialPrototype.hpp>
#include <Engine.Assets.Type/Material/PhysicsMaterial.hpp>
#include <Engine.Assets.Type/Material/AudioMaterial.hpp>

#include "TypedAssetHandle.hpp"

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
