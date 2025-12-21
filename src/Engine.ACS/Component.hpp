#pragma once

#include <Engine.Common/Wrapper.hpp>

namespace hg {
	class LogicComponent;
}

namespace hg {
	template <typename ComponentType_ = LogicComponent>
	class VolatileComponent final :
		public UniquePtr<ComponentType_, void(*)(ptr<LogicComponent>)> {
	public:
		template <class Type_>
		friend class VolatileComponent;

	public:
		using this_type = VolatileComponent<ComponentType_>;
		using component_type = ComponentType_;
		using base_type = UniquePtr<ComponentType_, void(*)(ptr<LogicComponent>)>;

	public:
		using base_type::base_type;

		VolatileComponent() = delete;

		constexpr explicit VolatileComponent(_Inout_ mref<ptr<ComponentType_>> component_) noexcept :
			base_type(component_, VolatileComponent<>::destroy) {}

		template <typename OtherComponentType_>
		constexpr explicit VolatileComponent(_Inout_ mref<VolatileComponent<OtherComponentType_>> other_) noexcept :
			base_type(static_cast<typename base_type::pointer>(other_.release()), VolatileComponent<>::destroy) {}

	public:
		static void destroy(ptr<LogicComponent> obj_);
	};
}
