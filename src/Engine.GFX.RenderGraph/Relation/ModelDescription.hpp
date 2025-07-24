#pragma once

#include <Engine.Reflect/Compile/TypeId.hpp>

#include "Description.hpp"

namespace hg::engine::render::graph {
	class ModelDescription final :
		public InheritMeta<ModelDescription, Description> {
	public:
		using this_type = ModelDescription;

	public:
		constexpr static type_id typeId = ::hg::refl::ctid<this_type>();

	public:
		ModelDescription() noexcept;

		~ModelDescription() override;

	protected:
		[[nodiscard]] bool isValueCompatible(
			const non_owning_rptr<const Description> other_
		) const noexcept override;
	};
}
