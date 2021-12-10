#pragma once
#include "ModelComponent.hpp"
#include "StaticGeometryAsset.hpp"

namespace ember {

    class StaticGeometryComponent :
        public ModelComponent {
    public:
        using this_type = StaticGeometryComponent;
        using underlying_type = ModelComponent;

        inline constexpr static component_type_id type_id { "StaticGeometryComponent"_typeId };

    public:
        StaticGeometryComponent(mref<CachedActorPointer> owner_, mref<ptr<ActorComponent>> parent_);

    private:
        StaticGeometryAsset _staticGeometry;

    public:
        [[nodiscard]] asset_guid getStaticGeometryGuid() const noexcept;

        [[nodiscard]] cref<StaticGeometryAsset> getStaticGeometryAsset() const noexcept;

        bool setStaticGeometryByAsset(cref<StaticGeometryAsset> asset_);
    };
}
