#pragma once

#include <Engine.Serialization/Access/__fwd.hpp>

#include "CachedActorPointer.hpp"
#include "LogicComponent.hpp"
#include "../Math/BuildIn.hpp"

namespace hg {
	/**
	 * Forward Declaration
	 */
	class Actor;

	// Question: Do we still need the HierarchyComponent do be available, or should we fuse it with the SceneComponent?
	class HierarchyComponent :
		public InheritMeta<HierarchyComponent, LogicComponent> {
	public:
		ACCESS_LAYOUT
		ACCESS_STRUCTURE

	public:
		using this_type = HierarchyComponent;
		using underlying_type = LogicComponent;

		constexpr static component_type_id typeId { "HierarchyComponent"_typeId };

	public:
		HierarchyComponent(
			mref<component_type_id> typeId_,
			mref<CachedActorPointer> owner_,
			mref<ptr<HierarchyComponent>> parent_
		);

		HierarchyComponent(mref<CachedActorPointer> owner_, mref<ptr<HierarchyComponent>> parent_);

		HierarchyComponent(mref<HierarchyComponent> other_) noexcept = default;

		HierarchyComponent(cref<HierarchyComponent>) = delete;

		~HierarchyComponent() override = default;

	private:
		CachedActorPointer _owner;

	public:
		[[nodiscard]] ptr<Actor> getOwner() const noexcept;

	private:
		ptr<HierarchyComponent> _parent;

	public:
		[[nodiscard]] ptr<Actor> getRootActor() const noexcept;

		[[nodiscard]] ptr<HierarchyComponent> getRootComponent() const noexcept;

		[[nodiscard]] ptr<HierarchyComponent> getParentComponent() const noexcept;

	private:
		math::Bounding _boundaries;

	public:
		[[nodiscard]] cref<math::Bounding> getBoundaries() const noexcept;

	private:
		Transform _localTransform;

	public:
		[[nodiscard]] cref<Transform> getLocalTransform() const noexcept;

		[[nodiscard]] ref<Transform> getLocalTransform() noexcept;

	public:
		[[nodiscard]] math::Location getUniverseLocation() const noexcept;

		[[nodiscard]] math::Rotator getUniverseRotator() const noexcept;

		[[nodiscard]] math::fmat4 getUniverseMatrix() const noexcept;
	};
}
