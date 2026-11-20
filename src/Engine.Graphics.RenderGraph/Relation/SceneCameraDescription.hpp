#pragma once

#include "Description.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Reflect;
#endif

namespace hg::engine::render::graph {
	class SceneCameraDescription final :
		public InheritMeta<SceneCameraDescription, Description> {
	public:
		using this_type = SceneCameraDescription;

	public:
		constexpr static type_id typeId = ::hg::refl::ctid<this_type>();

	public:
		SceneCameraDescription() noexcept;

		~SceneCameraDescription() override;

	protected:
		[[nodiscard]] bool isValueCompatible(
			const non_owning_rptr<const Description> other_
		) const noexcept override;
	};
}
