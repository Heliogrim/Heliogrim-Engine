#pragma once

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.ACS;
import Engine.Reflow;
#endif

namespace hg {
	class UIComponent :
		public InheritMeta<UIComponent, SceneComponent> {
	public:
		using this_type = UIComponent;
		using underlying_type = SceneComponent;

		constexpr static ComponentTypeId typeId { "UIComponent"_typeId };

	public:
		UIComponent(
			mref<ComponentGuid> guid_,
			mref<CachedActorPointer> owner_,
			mref<ptr<HierarchyComponent>> parent_
		) :
			InheritMeta(
				::hg::move(guid_),
				ComponentTypeId { typeId },
				std::move(owner_),
				std::move(parent_)
			) {}

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