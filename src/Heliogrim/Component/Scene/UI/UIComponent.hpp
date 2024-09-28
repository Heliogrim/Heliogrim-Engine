#pragma once

#include "../../SceneComponent.hpp"

#include <Engine.Reflow/Window/Window.hpp>

namespace hg {
	class UIComponent :
		public InheritMeta<UIComponent, SceneComponent> {
	public:
		using this_type = UIComponent;
		using underlying_type = SceneComponent;

		constexpr static component_type_id typeId { "UIComponent"_typeId };

	public:
		UIComponent(
			mref<CachedActorPointer> owner_,
			mref<ptr<HierarchyComponent>> parent_
		) :
			InheritMeta(component_type_id { typeId }, std::move(owner_), std::move(parent_)) {}

		~UIComponent() override = default;

	private:
		SharedPtr<engine::reflow::Window> _window;

	public:
		[[nodiscard]] cref<SharedPtr<engine::reflow::Window>> getWindow() const noexcept {
			return _window;
		}

		void setWindow(cref<SharedPtr<engine::reflow::Window>> window_) {
			_window = window_;
		}
	};
}
