#pragma once
#include <Engine.Common/__macro.hpp>
#include <Engine.Reflect/Compile/TypeId.hpp>

#include "Component.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::render::graph {
	class AnchorComponent final :
		public InheritMeta<AnchorComponent, Component> {
	public:
		using this_type = AnchorComponent;

	public:
		constexpr static type_id typeId = ::hg::refl::ctid<this_type>();

	public:
		AnchorComponent() noexcept;

		AnchorComponent(mref<string> anchorName_) noexcept;

		~AnchorComponent() noexcept override = default;

	private:
		string _name;

	public:
		[[nodiscard]] string_view getAnchorName() const noexcept;
	};
}
