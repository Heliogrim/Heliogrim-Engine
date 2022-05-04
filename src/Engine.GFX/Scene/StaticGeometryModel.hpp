#pragma once
#include <Engine.Assets/Types/StaticGeometry.hpp>
#include <Engine.Resource/Manage/Resource.hpp>

#include "SceneNodeModel.hpp"
#include "../Shader/CachedDiscreteBinding.hpp"

namespace ember::engine::gfx {

    class StaticGeometryModel final :
        public SceneNodeModel {
    public:
        using this_type = StaticGeometryModel;
        using underlying_type = SceneNodeModel;

        inline constexpr static type_id type_id { "StaticGeometryModel"_typeId };

    public:
        StaticGeometryModel(const ptr<SceneComponent> owner_);

        ~StaticGeometryModel() override final;

    private:
        void tidy();

    public:
        void create(const ptr<scene::Scene> scene_) override final;

        void update(const ptr<scene::Scene> scene_) override final;

        void destroy(const ptr<scene::Scene> scene_) override final;

    private:
        ptr<assets::StaticGeometry> _staticGeometryAsset = nullptr;
        ptr<res::Resource> _staticGeometryResource = nullptr;
        bool _streamable = false;

    public:
        [[nodiscard]] const ptr<assets::StaticGeometry> geometryAsset() const noexcept;

        [[nodiscard]] const ptr<res::Resource> geometryResource() const noexcept;

        [[nodiscard]] bool streamable() const noexcept;

    public:
        Buffer __tmp__instance;
        ptr<CachedDiscreteBinding> __tmp__cdb;

    public:
        [[nodiscard]] ModelBatch batch(const GraphicPassMask mask_) override;
    };
}
