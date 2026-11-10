#pragma once

#include <Engine.Common/Collection/DenseSet.hpp>
#include <Engine.Reflect/Compile/TypeId.hpp>

#include "Component.hpp"
#include "../Relation/Provision.hpp"

namespace hg::engine::render::graph {
	class ProviderComponent :
		public InheritMeta<ProviderComponent, Component> {
	public:
		using this_type = ProviderComponent;

	public:
		constexpr static type_id typeId = ::hg::refl::ctid<this_type>();

	public:
		ProviderComponent() noexcept = default;

		~ProviderComponent() noexcept override = default;

	private:
		DenseSet<Provision> _provided;

	public:
		[[nodiscard]] cref<DenseSet<Provision>> getProvided() const noexcept;

		void setProvided(mref<DenseSet<Provision>> provided_);
	};
}
