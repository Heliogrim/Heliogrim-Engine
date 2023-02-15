#pragma once
#include <Engine.Assets/Types/Geometry/StaticGeometry.hpp>
#include <Engine.Resource/Manage/Resource.hpp>

#include "GeometryModel.hpp"
#include "../Shader/CachedDiscreteBinding.hpp"
#include "../Renderer/__fwd.hpp"

namespace ember::engine::gfx {
    class SkyboxModel final :
        public GeometryModel {
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
        void create(const ptr<::ember::engine::scene::Scene> scene_) override final;

        void update(const ptr<::ember::engine::scene::Scene> scene_) override final;

        void destroy(const ptr<::ember::engine::scene::Scene> scene_) override final;

    public:
        [[nodiscard]] Vector<render::RenderDataToken> providedToken() const noexcept override;

    public:
        [[nodiscard]] ptr<cache::ModelBatch> batch(const ptr<render::RenderPassState> state_) override;

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
