#pragma once

#include <Engine.Common/Sal.hpp>
#include <Engine.Reflow.Uikit/Exp/Card.hpp>
#include <Engine.Reflow.Uikit/Molecule/Layout/VScrollBox.hpp>

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg {
	class Actor;
	class HierarchyComponent;
}

namespace hg::editor::ui {
	class PropertyController;
}

namespace hg::editor::ui::service {
	struct Property;
}

namespace hg::editor::ui {
	class PropertyView {
	public:
		PropertyView(
			ref<PropertyController> controller_,
			mref<SharedPtr<engine::reflow::uikit::Card>> main_,
			mref<std::weak_ptr<engine::reflow::uikit::VScrollBox>> container_
		);

	private:
		ref<PropertyController> _controller;

	public:
		SharedPtr<engine::reflow::uikit::Card> main;
		std::weak_ptr<engine::reflow::uikit::VScrollBox> container;

	public:
		void displayProperties(
			ref<const Vector<nmpt<const service::Property>>> properties_,
			ref<const Variant<nmpt<Actor>, nmpt<HierarchyComponent>>> data_
		);
	};

	/**/

	[[nodiscard]] extern UniquePtr<PropertyView> makePropertyView(_Inout_ ref<PropertyController> controller_);
}
