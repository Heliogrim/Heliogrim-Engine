#pragma once
#include <Engine.Assets/Types/SkeletalGeometry.hpp>
#include <Engine.Resource/Manage/Resource.hpp>

#include "GeometryModel.hpp"

namespace hg {
    class StaticGeometryComponent;
}

namespace hg::engine::gfx {
    class SkeletalGeometryModel :
        public InheritMeta<StaticGeometryComponent, GeometryModel> {
    public:
        SkeletalGeometryModel(const ptr<SceneComponent> owner_);

        ~SkeletalGeometryModel() override;

    public:
        [[nodiscard]] const ptr<StaticGeometryComponent> owner() const noexcept;

    public:
        void create(const ptr<scene::Scene> scene_) override final;

        void update(const ptr<scene::Scene> scene_) override final;

        void destroy(const ptr<scene::Scene> scene_) override final;

    public:
        void render(mref<nmpt<render::pipeline::IGCommandBuffer>> cmd_) const override;

    private:
        ptr<assets::SkeletalGeometry> _skeletalGeometry = nullptr;
        // TODO:
        ptr<StaticGeometryResource> _skeletalGeometryResource = nullptr;
        bool _streamable = false;
    };
}
