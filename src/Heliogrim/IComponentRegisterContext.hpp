#pragma once

#include <Engine.Common/Sal.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/__macro.hpp>

namespace hg {
	/**
	 * Forward Declaration
	 */
	class HierarchyComponent;
	class SceneComponent;

	class ComponentHierarchy;

	class macro_novtable IComponentRegisterContext {
	public:
		virtual void add(_In_ ptr<HierarchyComponent> component_) = 0;

		virtual void add(cref<ComponentHierarchy> hierarchy_) = 0;
	};
}
