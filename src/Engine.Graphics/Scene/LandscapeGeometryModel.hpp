#pragma once
#include <Engine.Assets/Types/LandscapeGeometry.hpp>

#include "SceneNodeModel.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Resource;
#endif

namespace hg::engine::graphics {
    class LandscapeGeometryModel :
        public InheritMeta<LandscapeGeometryModel, SceneNodeModel> {
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
