#pragma once
#include "GfxMaterialAsset.hpp"
#include "SceneComponent.hpp"
#include "StaticGeometryAsset.hpp"

namespace ember {

    class SkyboxComponent :
        public SceneComponent {
    public:
        using this_type = SkyboxComponent;
        using underlying_type = SceneComponent;

        inline constexpr static component_type_id typeId { "SkyboxComponent"_typeId };

    public:
        SkyboxComponent(
            mref<CachedActorPointer> owner_,
            mref<ptr<ActorComponent>> parent_
        );

        ~SkyboxComponent() = default;

    private:
        StaticGeometryAsset _skyboxGeometry;

    public:
        [[nodiscard]] cref<StaticGeometryAsset> getSkyboxGeometryAsset() const noexcept;

        bool setSkyboxGeometryByAsset(cref<StaticGeometryAsset> asset_);

    private:
        GfxMaterialAsset _skyboxMaterial;

    public:
        [[nodiscard]] cref<GfxMaterialAsset> getSkyboxMaterialAsset() const noexcept;

        bool setSkyboxMaterialByAsset(cref<GfxMaterialAsset> asset_);
    };
}
