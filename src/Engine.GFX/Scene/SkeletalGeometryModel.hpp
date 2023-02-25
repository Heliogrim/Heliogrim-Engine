#pragma once
#include <Engine.Assets/Types/SkeletalGeometry.hpp>
#include <Engine.Resource/Manage/Resource.hpp>

#include "SceneNodeModel.hpp"

namespace ember::engine::gfx {
    class SkeletalGeometryModel :
        public SceneNodeModel {
    public:
        SkeletalGeometryModel(const ptr<SceneComponent> owner_);

        ~SkeletalGeometryModel() override final = default;

    public:
        void create(const ptr<scene::Scene> scene_) override final;

        void update(const ptr<scene::Scene> scene_) override final;

        void destroy(const ptr<scene::Scene> scene_) override final;

    private:
        ptr<assets::SkeletalGeometry> _skeletalGeometry = nullptr;
        ptr<res::Resource> _skeletalGeometryResource = nullptr;
        bool _streamable = false;
    };
}
