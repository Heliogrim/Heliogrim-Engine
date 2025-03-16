#pragma once

#include <Engine.Common/Sal.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Collection/Vector.hpp>
#include <Engine.Common/Memory/MemoryPointer.hpp>
#include <Heliogrim/Actor/Actor.hpp>
#include <Heliogrim/Component/HierarchyComponent.hpp>

#include "Property.hpp"
#include "../UiService.hpp"

namespace hg::editor::ui::service {
	struct SceneHierarchyEntry;
}

namespace hg::editor::ui::service {
	class PropertyService :
		public UiService {
	public:
		using this_type = PropertyService;

	public:
		PropertyService();

		~PropertyService() override;

	private:
		Vector<UniquePtr<Property>> _registered;

	public:
		void registerProperty(mref<Property> property_);

		[[nodiscard]] Vector<nmpt<const Property>> fetchPropertiesFor(nmpt<const Actor> actor_) const;

		[[nodiscard]] Vector<nmpt<const Property>> fetchPropertiesFor(nmpt<const HierarchyComponent> component_) const;
	};
}
