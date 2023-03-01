#pragma once

#include <Engine.Common/Collection/CompactArray.hpp>

#include "ModelComponent.hpp"
#include "StaticGeometryAsset.hpp"
#include "GfxMaterialAsset.hpp"

namespace hg {
    class StaticGeometryComponent :
        public ModelComponent {
    public:
        using this_type = StaticGeometryComponent;
        using underlying_type = ModelComponent;

        inline constexpr static component_type_id typeId { "StaticGeometryComponent"_typeId };

    public:
        StaticGeometryComponent(mref<CachedActorPointer> owner_, mref<ptr<ActorComponent>> parent_);

        ~StaticGeometryComponent() = default;

    private:
        StaticGeometryAsset _staticGeometry;

    public:
        [[nodiscard]] asset_guid getStaticGeometryGuid() const noexcept;

        [[nodiscard]] cref<StaticGeometryAsset> getStaticGeometryAsset() const noexcept;

        bool setStaticGeometryByAsset(cref<StaticGeometryAsset> asset_);

    private:
        CompactArray<GfxMaterialAsset> _overrideMaterials;

    public:
        [[nodiscard]] cref<CompactArray<GfxMaterialAsset>> overrideMaterials() const noexcept;
    };
}
