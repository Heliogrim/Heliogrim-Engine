#pragma once
#include <Engine.GFX/Scene/SceneNodeModel.hpp>

namespace hg::engine::reflow {
    class UISceneModel final :
        public InheritMeta<UISceneModel, gfx::SceneNodeModel> {
    public:
        using this_type = UISceneModel;
        using underlying_type = gfx::SceneNodeModel;

        inline constexpr static type_id typeId { "UISceneModel"_typeId };

    public:
        UISceneModel(const ptr<SceneComponent> owner_);

        ~UISceneModel() override final = default;

    public:
        void create(const ptr<scene::Scene> scene_) override;

        void update(const ptr<scene::Scene> scene_) override;

        void destroy(const ptr<scene::Scene> scene_) override;

    public:
        [[nodiscard]] ref<class Window> getWindow() const noexcept;
    };
}
