#pragma once
#include <Engine.Assets/Types/Geometry/StaticGeometry.hpp>
#include <Engine.Resource/Manage/Resource.hpp>
#include <Engine.GFX.Loader/Geometry/StaticGeometryResource.hpp>
#include <Engine.Common/Concurrent/SharedMemoryReference.hpp>

#include "GeometryModel.hpp"

namespace hg::engine::gfx::scene {
    class StaticGeometryModel final :
        public InheritMeta<StaticGeometryModel, GeometryModel> {
    public:
        using this_type = StaticGeometryModel;
        using underlying_type = SceneNodeModel;

        inline constexpr static type_id typeId { "StaticGeometryModel"_typeId };

    public:
        StaticGeometryModel(const ptr<SceneComponent> owner_);

        ~StaticGeometryModel() override;

    private:
        void tidy();

    public:
        void create(const ptr<::hg::engine::scene::Scene> scene_) override;

        void update(const ptr<::hg::engine::scene::Scene> scene_) override;

        void destroy(const ptr<::hg::engine::scene::Scene> scene_) override;

    public:
        void capture(nmpt<render::MeshCaptureInterface> mci_) const noexcept override;

    private:
        ptr<assets::StaticGeometry> _staticGeometryAsset = nullptr;
        smr<StaticGeometryResource> _staticGeometryResource;

    public:
        [[nodiscard]] const ptr<assets::StaticGeometry> geometryAsset() const noexcept;

        [[nodiscard]] cref<smr<StaticGeometryResource>> geometryResource() const noexcept;
    };
}
