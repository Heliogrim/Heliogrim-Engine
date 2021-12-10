#pragma once
#include <Engine.Assets/Types/StaticGeometry.hpp>
#include <Engine.Resource/Manage/Resource.hpp>

#include "SceneNodeModel.hpp"

namespace ember::engine::gfx {

    class StaticGeometryModel final :
        public SceneNodeModel {
    public:
        StaticGeometryModel(const ptr<SceneComponent> owner_);

        ~StaticGeometryModel() override final = default;

    public:
        void create(const ptr<scene::Scene> scene_) override final;

        void update(const ptr<scene::Scene> scene_) override final;

        void destroy(const ptr<scene::Scene> scene_) override final;

    private:
        ptr<assets::StaticGeometry> _staticGeometry = nullptr;
        ptr<res::Resource> _staticGeometryResource = nullptr;
        bool _streamable = false;
    };
}
