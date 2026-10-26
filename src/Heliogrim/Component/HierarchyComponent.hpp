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

		constexpr static ComponentTypeId typeId { "HierarchyComponent"_typeId };

	public:
		HierarchyComponent(
			mref<ComponentGuid> guid_,
			mref<ComponentTypeId> typeId_,
			mref<CachedActorPointer> owner_,
			mref<ptr<HierarchyComponent>> parent_
		);

		HierarchyComponent(mref<ComponentGuid> guid_, mref<CachedActorPointer> owner_, mref<ptr<HierarchyComponent>> parent_);

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

		/**
		 * Places this component at the given pose, which is expressed in the outer (universe) space.
		 *
		 * @details Contract: afterwards `getUniverseLocation()` and `getUniverseRotator()` yield the provided pose.
		 *	For a component without parent the pose is stored as is. For a child, the local transform is derived
		 *	by undoing the parent's universe transform, so it depends on the parent's location, rotation and scale.
		 *
		 * @param outerLocation_ The desired location in universe space.
		 * @param outerRotator_ The desired rotation in universe space.
		 */
		void setRelativeLocationAndRotator(mref<math::Location> outerLocation_, mref<math::Rotator> outerRotator_) noexcept;

	public:
		/**
		 * Universe space pose of this component, derived from the whole parent chain.
		 */
		struct UniverseTransform {
			math::Location location;
			math::Rotator rotator;
			math::fmat4 matrix;
		};

		/**
		 * Walks the parent chain once and returns location, rotator and matrix together.
		 *
		 * @details Prefer this over calling the individual getters in a row, as each of them is a full chain walk.
		 */
		[[nodiscard]] UniverseTransform getUniverseTransform() const noexcept;

		[[nodiscard]] math::Location getUniverseLocation() const noexcept;

		[[nodiscard]] math::Rotator getUniverseRotator() const noexcept;

		[[nodiscard]] math::fmat4 getUniverseMatrix() const noexcept;
	};
}
