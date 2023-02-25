#pragma once
#include <Engine.Assets/Types/LandscapeGeometry.hpp>
#include <Engine.Resource/Manage/Resource.hpp>

#include "SceneNodeModel.hpp"

namespace ember::engine::gfx {
    class LandscapeGeometryModel :
        public SceneNodeModel {
    public:
        LandscapeGeometryModel(const ptr<SceneComponent> owner_);

        ~LandscapeGeometryModel() override final = default;

    public:
        void create(const ptr<scene::Scene> scene_) override final;

        void update(const ptr<scene::Scene> scene_) override final;

        void destroy(const ptr<scene::Scene> scene_) override final;

    private:
        ptr<assets::LandscapeGeometry> _landscapeGeometry = nullptr;
        ptr<res::Resource> _landscapeGeometryResource = nullptr;
        bool _streamable = false;
    };
}
