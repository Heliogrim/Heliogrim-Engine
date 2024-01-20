#pragma once
#include <Engine.Assets/Types/SkeletalGeometry.hpp>
#include <Engine.GFX.Loader/Geometry/StaticGeometryResource.hpp>
#include <Engine.GFX.Scene.Model/GeometryModel.hpp>
#include <Engine.Resource/Manage/Resource.hpp>

namespace hg {
    class StaticGeometryComponent;
}

namespace hg::engine::gfx {
    class SkeletalGeometryModel :
        public InheritMeta<StaticGeometryComponent, scene::GeometryModel> {
    public:
        SkeletalGeometryModel(const ptr<SceneComponent> owner_);

        ~SkeletalGeometryModel() override;

    public:
        void create(const ptr<render::RenderSceneSystem> system_) override;

        void update(const ptr<render::RenderSceneSystem> system_) override;

        void destroy(const ptr<render::RenderSceneSystem> system_) override;

    public:
        void capture(nmpt<render::MeshCaptureInterface> mci_) const noexcept override;

    private:
        ptr<assets::SkeletalGeometry> _skeletalGeometry = nullptr;
        // TODO:
        ptr<StaticGeometryResource> _skeletalGeometryResource = nullptr;
        bool _streamable = false;
    };
}
