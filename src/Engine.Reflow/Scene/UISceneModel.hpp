#pragma once
#include <Engine.Reflect/Inherit/InheritMeta.hpp>
#include <Engine.Render.Scene/RenderSceneSystemModel.hpp>

namespace hg::engine::reflow {
    class UISceneModel final :
        public InheritMeta<UISceneModel, engine::render::RenderSceneSystemModel> {
    public:
        using this_type = UISceneModel;

        inline constexpr static type_id typeId { "UISceneModel"_typeId };

    public:
        UISceneModel(const ptr<SceneComponent> owner_);

        UISceneModel(mref<this_type>) noexcept = default;

        ~UISceneModel() override = default;

    public:
        ref<this_type> operator=(mref<this_type>) noexcept = default;

    public:
        void create(const ptr<engine::render::RenderSceneSystem> system_) override;

        void update(const ptr<engine::render::RenderSceneSystem> system_) override;

        void destroy(const ptr<engine::render::RenderSceneSystem> system_) override;

    public:
        [[nodiscard]] ref<class Window> getWindow() const noexcept;
    };
}
