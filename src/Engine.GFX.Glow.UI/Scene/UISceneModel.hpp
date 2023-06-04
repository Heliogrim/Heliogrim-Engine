#pragma once
#include <Engine.GFX/Scene/SceneNodeModel.hpp>
#include <Engine.GFX.Renderer/__fwd.hpp>

#include "Engine.Reflow/Window/__fwd.hpp"

namespace hg::engine::gfx::glow::ui {
    class UISceneModel final :
        public SceneNodeModel {
    public:
        using this_type = UISceneModel;
        using underlying_type = SceneNodeModel;

        inline constexpr static type_id typeId { "UISceneModel"_typeId };

    public:
        UISceneModel(const ptr<SceneComponent> owner_);

        ~UISceneModel() override final = default;

    public:
        void create(const ptr<engine::scene::Scene> scene_) override;

        void update(const ptr<engine::scene::Scene> scene_) override;

        void destroy(const ptr<engine::scene::Scene> scene_) override;

    public:
        Vector<gfx::render::RenderDataToken> providedToken() const noexcept override;

    public:
        ptr<cache::ModelBatch> batch(const ptr<gfx::render::RenderPassState> state_) override;

    public:
        [[nodiscard]] sptr<reflow::Window> getWindow() const noexcept;
    };
}
