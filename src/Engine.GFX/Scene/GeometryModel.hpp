#pragma once
#include <Engine.Common/Collection/CompactArray.hpp>
#include <Engine.GFX.Loader/Material/MaterialResource.hpp>

#include "SceneNodeModel.hpp"

namespace hg::engine::gfx {
    class GeometryModel :
        public SceneNodeModel {
    public:
        using this_type = GeometryModel;
        using underlying_type = SceneNodeModel;

    public:
        GeometryModel(const ptr<SceneComponent> owner_);

        ~GeometryModel() override = default;

    protected:
        CompactArray<smr<MaterialResource>> _overrideMaterials;

    public:
        [[nodiscard]] bool hasOverrideMaterials() const noexcept;

        [[nodiscard]] cref<CompactArray<smr<MaterialResource>>> overrideMaterials() const noexcept;

        [[nodiscard]] virtual cref<smr<MaterialResource>> material(const u32 index_) const noexcept;
    };
}
