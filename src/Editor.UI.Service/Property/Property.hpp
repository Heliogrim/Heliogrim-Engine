#pragma once

#include <Engine.ACS/ComponentTypeId.hpp>
#include <Engine.Common/Variant.hpp>
#include <Engine.Common/Memory/MemoryPointer.hpp>
#include <Engine.Reflect/Meta/MetaClass.hpp>
#include <Engine.Reflow/Widget/Widget.hpp>
#include <Heliogrim/Actor/Actor.hpp>

namespace hg::editor::ui::service {
	struct Property {
		using selector_fn_arg = ref<const Variant<nmpt<const Actor>, nmpt<const HierarchyComponent>>>;
		using selector_fn = std::function<bool(selector_fn_arg)>;
		using view_fn_arg = ref<const Variant<nmpt<Actor>, nmpt<HierarchyComponent>>>;

		String debugName;
		Variant<nmpt<const ActorClass>, component_type_id, selector_fn> selector;
		std::function<SharedPtr<engine::reflow::Widget>(view_fn_arg)> view;
	};
}
