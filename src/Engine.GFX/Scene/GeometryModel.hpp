#pragma once
#include <Engine.Common/Collection/CompactArray.hpp>
#include <Engine.GFX.Loader/Material/MaterialResource.hpp>
#include <Engine.GFX.Render.Subpass/Mesh/MeshCaptureInterface.hpp>

#include "SceneNodeModel.hpp"

namespace hg::engine::gfx {
    class GeometryModel :
        public InheritMeta<GeometryModel, SceneNodeModel> {
    public:
        using this_type = GeometryModel;
        using underlying_type = SceneNodeModel;

    protected:
        GeometryModel(const ptr<SceneComponent> owner_);

    public:
        ~GeometryModel() override = default;

    protected:
        CompactArray<smr<MaterialResource>> _overrideMaterials;

    public:
        [[nodiscard]] bool hasOverrideMaterials() const noexcept;

        [[nodiscard]] cref<CompactArray<smr<MaterialResource>>> overrideMaterials() const noexcept;

        [[nodiscard]] virtual cref<smr<MaterialResource>> material(const u32 index_) const noexcept;

    public:
        virtual void capture(nmpt<render::MeshCaptureInterface> mci_) const noexcept = 0;
    };
}
