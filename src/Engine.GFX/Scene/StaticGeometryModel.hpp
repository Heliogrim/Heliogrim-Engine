#pragma once
#include <Engine.Assets/Types/StaticGeometry.hpp>
#include <Engine.Resource/Manage/Resource.hpp>

#include "GeometryModel.hpp"
#include "../Shader/CachedDiscreteBinding.hpp"
#include "../Renderer/__fwd.hpp"

namespace ember::engine::gfx {

    class StaticGeometryModel final :
        public GeometryModel {
    public:
        using this_type = StaticGeometryModel;
        using underlying_type = SceneNodeModel;

        inline constexpr static type_id typeId { "StaticGeometryModel"_typeId };

    public:
        StaticGeometryModel(const ptr<SceneComponent> owner_);

        ~StaticGeometryModel() override final;

    private:
        void tidy();

    public:
        void create(const ptr<scene::Scene> scene_) override final;

        void update(const ptr<scene::Scene> scene_) override final;

        void destroy(const ptr<scene::Scene> scene_) override final;

    public:
        [[nodiscard]] Vector<render::RenderDataToken> providedToken() const noexcept override;

    public:
        [[nodiscard]] ptr<cache::ModelBatch> batch(const ptr<render::RenderPassState> state_) override;

    private:
        ptr<assets::StaticGeometry> _staticGeometryAsset = nullptr;
        ptr<res::Resource> _staticGeometryResource = nullptr;
        bool _streamable = false;

    public:
        [[nodiscard]] const ptr<assets::StaticGeometry> geometryAsset() const noexcept;

        [[nodiscard]] const ptr<res::Resource> geometryResource() const noexcept;

        [[nodiscard]] bool streamable() const noexcept;
    };
}
