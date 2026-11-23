#pragma once

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Reflect;
import Engine.Render.Scene;
#endif

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
