#pragma once
#include <Engine.Assets/Types/Geometry/StaticGeometry.hpp>
#include <Engine.Resource/Manage/Resource.hpp>
#include <Engine.GFX.Loader/Geometry/StaticGeometryResource.hpp>
#include <Engine.GFX.Renderer/__fwd.hpp>
#include <Engine.Common/Concurrent/SharedMemoryReference.hpp>

#include "GeometryModel.hpp"
#include "../Shader/CachedDiscreteBinding.hpp"

namespace hg::engine::gfx {
    class StaticGeometryModel final :
        public InheritMeta<StaticGeometryModel, GeometryModel> {
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
        void create(const ptr<::hg::engine::scene::Scene> scene_) override final;

        void update(const ptr<::hg::engine::scene::Scene> scene_) override final;

        void destroy(const ptr<::hg::engine::scene::Scene> scene_) override final;

    public:
        [[nodiscard]] Vector<render::RenderDataToken> providedToken() const noexcept override;

    public:
        [[nodiscard]] ptr<cache::ModelBatch> batch(const ptr<render::RenderPassState> state_) override;

        void render(mref<nmpt<render::pipeline::IGCommandBuffer>> cmd_) const override;

    private:
        ptr<assets::StaticGeometry> _staticGeometryAsset = nullptr;
        smr<StaticGeometryResource> _staticGeometryResource;
        bool _streamable = false;

    public:
        [[nodiscard]] const ptr<assets::StaticGeometry> geometryAsset() const noexcept;

        [[nodiscard]] cref<smr<StaticGeometryResource>> geometryResource() const noexcept;

        [[nodiscard]] bool streamable() const noexcept;
    };
}
