#pragma once
#include <Engine.Common/Collection/CompactArray.hpp>

#include "SceneNodeModel.hpp"
#include "../Resource/MaterialResource.hpp"

namespace ember::engine::gfx {

    class GeometryModel :
        public SceneNodeModel {
    public:
        using this_type = GeometryModel;
        using underlying_type = SceneNodeModel;

    public:
        GeometryModel(const ptr<SceneComponent> owner_);

        ~GeometryModel() override = default;

    protected:
        CompactArray<ptr<MaterialResource>> _overrideMaterials;

    public:
        [[nodiscard]] bool hasOverrideMaterials() const noexcept;

        [[nodiscard]] cref<CompactArray<ptr<MaterialResource>>> overrideMaterials() const noexcept;

        [[nodiscard]] virtual const ptr<MaterialResource> material(const u32 index_) const noexcept;
    };
}
