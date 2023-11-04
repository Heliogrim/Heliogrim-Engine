#pragma once
#include <Engine.Assets/Types/Geometry/StaticGeometry.hpp>
#include <Engine.Resource/Manage/Resource.hpp>

#include "GeometryModel.hpp"
#include "../Shader/CachedDiscreteBinding.hpp"

namespace hg::engine::gfx {
    class SkyboxModel final :
        public InheritMeta<SkyboxModel, GeometryModel> {
    public:
        using this_type = SkyboxModel;
        using underlying_type = SceneNodeModel;

        inline constexpr static type_id typeId { "SkyboxModel"_typeId };

    public:
        SkyboxModel(const ptr<SceneComponent> owner_);

        ~SkyboxModel() override final;

    private:
        void tidy();

    public:
        void create(const ptr<::hg::engine::scene::Scene> scene_) override final;

        void update(const ptr<::hg::engine::scene::Scene> scene_) override final;

        void destroy(const ptr<::hg::engine::scene::Scene> scene_) override final;

    public:
        void render(mref<nmpt<render::cmd::RenderCommandBuffer>> cmd_) const override;

    private:
        ptr<assets::StaticGeometry> _skyboxGeometryAsset = nullptr;
        smr<resource::ResourceBase> _skyboxGeometryResource = {};
        bool _streamable = false;

    public:
        [[nodiscard]] const ptr<assets::StaticGeometry> geometryAsset() const noexcept;

        [[nodiscard]] cref<smr<resource::ResourceBase>> geometryResource() const noexcept;

        [[nodiscard]] bool streamable() const noexcept;
    };
}
