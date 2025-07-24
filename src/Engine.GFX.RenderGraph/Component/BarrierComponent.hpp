#pragma once
#include <Engine.Common/String.hpp>
#include <Engine.Reflect/Compile/TypeId.hpp>

#include "Component.hpp"

namespace hg::engine::render::graph {
	class BarrierComponent final :
		public InheritMeta<BarrierComponent, Component> {
	public:
		using this_type = BarrierComponent;

	public:
		constexpr static type_id typeId = ::hg::refl::ctid<this_type>();

	public:
		BarrierComponent() noexcept = default;

		BarrierComponent(mref<string> barrierName_) noexcept;

		~BarrierComponent() noexcept override = default;

	private:
		string _name;

	public:
		[[nodiscard]] string_view getBarrierName() const noexcept;

		void setBarrierName(string_view barrierName_);
	};
}
