#pragma once
#include "SceneComponent.hpp"

#include <Engine.Reflow/Window/Window.hpp>

namespace ember {
    class UIComponent :
        public SceneComponent {
    public:
        using this_type = UIComponent;
        using underlying_type = SceneComponent;

        inline constexpr static component_type_id typeId { "UIComponent"_typeId };

    public:
        UIComponent(
            mref<CachedActorPointer> owner_,
            mref<ptr<ActorComponent>> parent_
        ) :
            SceneComponent(component_type_id { typeId }, _STD move(owner_), _STD move(parent_)) {}

        ~UIComponent() = default;

    private:
        sptr<engine::reflow::Window> _window;

    public:
        [[nodiscard]] cref<sptr<engine::reflow::Window>> getWindow() const noexcept {
            return _window;
        }

        void setWindow(cref<sptr<engine::reflow::Window>> window_) {
            _window = window_;
        }
    };
}
