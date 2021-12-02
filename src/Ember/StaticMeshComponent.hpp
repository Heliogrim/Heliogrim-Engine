#pragma once
#include "ModelComponent.hpp"
#include "StaticMeshAsset.hpp"

namespace ember {

    class StaticMeshComponent :
        public ModelComponent {
    public:
        using this_type = StaticMeshComponent;
        using underlying_type = ModelComponent;

    public:
        StaticMeshComponent(mref<CachedActorPointer> owner_, mref<ptr<ActorComponent>> parent_);

    private:
        asset_guid _staticMesh;

    public:
        [[nodiscard]] asset_guid getStaticMeshGuid() const noexcept;

        [[nodiscard]] StaticMeshAsset getStaticMeshAsset() const noexcept;

        bool setStaticMeshByGuid(cref<asset_guid> guid_);

        bool setStaticMeshByAsset(cref<StaticMeshAsset> asset_);
    };
}
