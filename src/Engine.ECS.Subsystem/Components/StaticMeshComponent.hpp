#pragma once

#include <Engine.Assets/AssetGuid.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.ECS/ComponentTypeId.hpp>
#include <Engine.ECS/Traits.hpp>
#include <Engine.Proxy/SceneProxiedRef.hpp>

#include "SceneComponent.hpp"

namespace ember::engine::ecs::subsystem {

    class StaticMeshComponent :
        public SceneComponent {
    public:
        constexpr static component_type_id type_id { "StaticMeshComponent"_typeId };

    public:
        StaticMeshComponent() noexcept = default;

    public:
        void materialize(_In_ cref<entity_guid> entity_);

        void dematerialize(_In_ cref<entity_guid> entity_);

    private:
        asset_guid _meshGuid;

    public:
        [[nodiscard]] cref<asset_guid> mesh() const noexcept;

    private:
        asset_guid _materialGuid;

    public:
        [[nodiscard]] cref<asset_guid> material() const noexcept;

    private:
        sptr<proxy::SceneProxiedRef> _proxy = nullptr;
    };
}
