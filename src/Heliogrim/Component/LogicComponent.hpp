#pragma once

#include <Engine.ACS/ComponentGuid.hpp>
#include <Engine.ACS/ComponentLabel.hpp>
#include <Engine.ACS/ComponentTypeId.hpp>
#include <Engine.Reflect/Inherit/InheritBase.hpp>
#include <Engine.Serialization/Access/__fwd.hpp>

namespace hg {
	class LogicComponent :
		public InheritBase<LogicComponent> {
	public:
		ACCESS_LAYOUT
		ACCESS_STRUCTURE

	public:
		using this_type = LogicComponent;
		using underlying_type = InheritBase<LogicComponent>;

	protected:
		LogicComponent(mref<ComponentGuid> guid_, mref<ComponentTypeId> typeId_) noexcept;

	public:
		~LogicComponent() override = default;

	private:
		ComponentTypeId _typeId;
		ComponentGuid _guid;
		ComponentLabel _label;

	public:
		[[nodiscard]] cref<ComponentTypeId> getTypeId() const noexcept;

		[[nodiscard]] ref<const ComponentGuid> guid() const noexcept;

		[[nodiscard]] ref<const ComponentLabel> label() const noexcept;

		void setLabel(mref<ComponentLabel> label_);
	};
}
