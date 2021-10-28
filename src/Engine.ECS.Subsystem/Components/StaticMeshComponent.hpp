#pragma once

#include <Engine.Assets/AssetGuid.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.ECS/ComponentTypeId.hpp>
#include <Engine.Proxy/StaticModelSceneProxy.hpp>

namespace ember::engine::ecs::subsystem {

	class StaticMeshComponent {
	public:
		constexpr static component_type_id type_id{"StaticMeshComponent"_typeId};

	public:
		StaticMeshComponent() noexcept = default;

	private:
		asset_guid _meshGuid;

	public:
		[[nodiscard]] cref<asset_guid> mesh() const noexcept;

	private:
		asset_guid _materialGuid;

	public:
		[[nodiscard]] cref<asset_guid> material() const noexcept;

	private:
		sptr<proxy::StaticModelSceneProxy> _proxy = nullptr;
	};
}