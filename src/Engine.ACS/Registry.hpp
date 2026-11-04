#pragma once

#include <vector>
#include <Engine.Common/Make.hpp>
#include <Engine.Common/Optional.hpp>
#include <Engine.Common/Collection/DenseMap.hpp>
#include <Engine.Common/Functional/FnRef.hpp>
#include <Engine.Reflect/TypeId.hpp>
#include <Engine.Reflect/Inherit/Concept.hpp>
#include <Heliogrim/Actor/ActorGuid.hpp>

#include "ActorPoolWrapper.hpp"
#include "ComponentGuid.hpp"
#include "ComponentTypeId.hpp"
#include "Housekeeping.hpp"
#include "Pool.hpp"
#include "PoolWrapper.hpp"
#include "Registry.hpp"
#include "Traits.hpp"
#include "Archetype/LayoutInfo.hpp"
#include "Cluster/ClusterRegion.hpp"
#include "Cluster/RelocationBarrier.hpp"
#include "Cluster/Span.hpp"
#include "Handle/HandleTable.hpp"

namespace hg {
	/**
	 * Forward Declaration
	 */
	class Actor;
	class ActorClass;
	class ActorInitializer;
	class MetaClass;
}

namespace hg::engine::acs {
	namespace detail {
		// Whether a type declares its own meta class, rather than inheriting its base's.
		template <typename Type_>
		struct own_meta : std::false_type {};

		template <typename Type_> requires requires { typename Type_::template meta_class<Type_>; }
		struct own_meta<Type_> :
			std::bool_constant<std::is_same_v<typename Type_::template meta_class<>, typename Type_::template meta_class<Type_>>> {};
	}

	class Registry {
	public:
		friend class ::hg::engine::acs::Housekeeping;

	public:
		using size_type = size_t;

		template <typename PooledType_>
		using ComponentPool = Pool<ComponentGuid, PooledType_, invalid_component_guid>;

		using ComponentPoolWrapperBase = PoolWrapperBase<ComponentGuid>;

		template <typename PooledType_>
		using ComponentPoolWrapper = PoolWrapper<PooledType_, ComponentGuid, invalid_component_guid>;

		template <typename PooledType_>
		using ActorPool = Pool<ActorGuid, PooledType_, invalid_actor_guid>;

	public:
		explicit Registry();

		/**
		 * Destructor
		 *
		 * @author Julius
		 * @date 29.10.2021
		 */
		~Registry();

		#pragma region Component Pooling

	private:
		DenseMap<type_id, ptr<ComponentPoolWrapperBase>> _pools;

	protected:
		void removeActorComponent(ref<const ComponentGuid> guid_, ref<const ComponentTypeId> typeId_);

	public:
		/**
		 * Pool Operations
		 */

		/**
		 * Gets or create pool
		 *
		 * @tparam ComponentType_ Type of the value to get a pool of.
		 *
		 * @returns The existing or create pool.
		 */
		template <typename ComponentType_>
		[[nodiscard]] ComponentPool<ComponentType_>& getOrCreateComponentPool() {

			using pool_type = ComponentPool<ComponentType_>;
			using wrapper_type = ComponentPoolWrapper<ComponentType_>;

			auto ptr = &pool_type::getOrCreate();

			// Temporary
			constexpr auto typeId = refl::TypeId<ComponentType_>();
			auto mapped = _pools[typeId];

			if (mapped == nullptr) {
				_pools[typeId] = new wrapper_type(ptr);
			}

			return *ptr;
		}

		/**
		 * Gets the pool
		 *
		 * @tparam ComponentType_ Type of the value to get a pool of.
		 *
		 * @returns A reference to a pool of value type.
		 */
		template <typename ComponentType_>
		[[nodiscard]] ComponentPool<ComponentType_>& pool() {

			using pool_type = ComponentPool<ComponentType_>;
			auto* ptr = pool_type::get();

			return *ptr;
		}

	public:
		/**
		 * Unsafe get of a value reference
		 *
		 * @tparam ComponentType_ Type of the value.
		 * @param  key_ The key.
		 *
		 * @returns A reference to a value.
		 */
		template <typename ComponentType_>
		ComponentType_& unsafe_get(const ComponentGuid& key_) {
			return pool<ComponentType_>().unsafe_get(key_);
		}

		/**
		 * Gets a value pointer using the given key
		 *
		 * @tparam ComponentType_ Type of the value to get.
		 * @param  key_ The key.
		 *
		 * @returns Null if it fails, else a pointer to a value.
		 */
		template <typename ComponentType_>
		ComponentType_* get(const ComponentGuid& key_) {
			return pool<ComponentType_>().get(key_);
		}

	public:
		template <class ValueType_, typename... Args_>
		[[nodiscard]] VolatileComponent<ValueType_> acquireActorComponent(ref<const ComponentGuid> guid_, Args_&&... args_) {

			auto& pool = getOrCreateComponentPool<ValueType_>();
			auto result = pool.emplace(guid_, ComponentGuid { guid_ }, std::forward<Args_>(args_)...);

			return VolatileComponent<ValueType_> { result.second ? result.first : nullptr };
		}

		void releaseActorComponent(mref<VolatileComponent<>> component_);

		template <ClassHasMeta ValueType_>
		void releaseActorComponent(mref<VolatileComponent<ValueType_>> component_) {
			releaseActorComponent(static_cast<mref<VolatileComponent<>>>(component_));
		}

		#pragma endregion
		#pragma region Actor Pooling

	private:
		DenseMap<type_id, ptr<ActorPoolWrapperBase>> _actorPools;

	public:
		template <typename ActorType_>
		const ptr<ActorPool<ActorType_>> getOrCreateActorPool() {

			using pool_type = ActorPool<ActorType_>;
			using wrapper_type = ActorPoolWrapper<ActorType_>;

			// TODO: Change static lifetime to scoped one
			auto* const pool = &pool_type::getOrCreate();

			constexpr auto typeId = refl::TypeId<ActorType_>();
			const auto* const mapped { _actorPools[typeId] };

			if (mapped == nullptr) {
				_actorPools[typeId] = new wrapper_type(pool);
			}

			return pool;
		}

	public:
		template <ClassHasMeta ActorType_> requires std::derived_from<ActorType_, Actor>
		[[nodiscard]] ptr<ActorType_> createActor(cref<ActorGuid> guid_, cref<ActorInitializer> initializer_) noexcept {

			hackActorInit(initializer_, guid_);

			auto* const pool { getOrCreateActorPool<ActorType_>() };
			pool->insert(guid_, initializer_);

			auto* actor { pool->get(guid_) };
			actor->unsafe_set_guid(guid_);

			return actor;
		}

		template <ClassHasMeta ActorType_> requires std::derived_from<ActorType_, Actor>
		[[nodiscard]] ptr<ActorType_> createActor(cref<ActorInitializer> initializer_) noexcept {
			return createActor<ActorType_>(generate_actor_guid(), initializer_);
		}

		[[nodiscard]] ptr<Actor> createActor(
			const ptr<const MetaClass> actorClass_,
			cref<ActorInitializer> initializer_
		);

		[[nodiscard]] ptr<Actor> createActor(
			const ptr<const MetaClass> actorClass_,
			cref<ActorInitializer> initializer_,
			std::nothrow_t
		) noexcept;

	public:
		[[nodiscard]] ptr<Actor> createActor(cref<ActorInitializer> initializer_) noexcept;

		void destroyActor(mref<ptr<Actor>> actor_);

	private:
		void hackActorInit(cref<ActorInitializer> initializer_, cref<ActorGuid> guid_) const noexcept;

		#pragma endregion
		#pragma region Archetype Clusters

	private:
		DenseMap<type_id, ptr<const ActorClass>> _classes;

		struct Space {
			explicit Space(SpaceId id_) :
				handles(id_) {}

			HandleTable handles;
			// Declared after `handles`: regions release their handles on destruction.
			DenseMap<ptr<const ActorClass>, uptr<ClusterRegion>> regions;
		};

		// Spaces with regions, including detachedSpace once used.
		DenseMap<SpaceId, uptr<Space>> _spaces;

		struct Location {
			ptr<Space> space;
			ptr<ClusterRegion> region;
			std::byte* slot;
			Handle handle;
		};

		[[nodiscard]] Location locate(cref<Actor> actor_) const;

		[[nodiscard]] ref<ClusterRegion> regionFor(SpaceId space_, ptr<const ActorClass> class_);

		void forEachSpan(
			const void* tag_,
			ptr<const MetaClass> meta_,
			FnRef<void(std::byte* base_, u32 count_, u32 stride_)> fn_
		) const;

		// Housekeeping: retired slots become claimable again.
		void reclaimSlots();

	public:
		/**
		 * Adds an archetype class to the runtime lookup; idempotent.
		 *
		 * @returns false if `class_` is null or another class holds its type id
		 */
		bool registerActorClass(ptr<const ActorClass> class_) noexcept;

		[[nodiscard]] Opt<ptr<const ActorClass>> findActorClass(type_id typeId_) const noexcept;

		/**
		 * Acquires a space for cluster regions (one per level).
		 */
		[[nodiscard]] SpaceId acquireSpace();

		/**
		 * Returns every chunk and the handle table of a space. Slot contents must be destroyed already.
		 */
		void releaseSpace(SpaceId space_);

		[[nodiscard]] bool hasSpace(SpaceId space_) const noexcept;

		/**
		 * @returns the region of `class_` in `space_`, or nullptr if none exists
		 */
		[[nodiscard]] ptr<const ClusterRegion> region(SpaceId space_, ptr<const ActorClass> class_) const noexcept;

		/**
		 * Allocates and constructs `count_` archetype actors in `space_`, one construct call per contiguous run.
		 */
		[[nodiscard]] std::vector<ptr<Actor>> createActors(ptr<const ActorClass> class_, size_t count_, SpaceId space_);

		/**
		 * Relocates an archetype actor into a region of `space_`.
		 *
		 * @returns the actor's new address, or `actor_` if it is in that space already
		 */
		[[nodiscard]] ptr<Actor> moveActor(ptr<Actor> actor_, SpaceId space_);

		/**
		 * Runs the destructors and releases the handle now; the slot is reused only after housekeeping.
		 */
		void destroyArchetypeActor(ptr<Actor> actor_);

		/**
		 * @returns the handle of an archetype actor, or an invalid handle
		 */
		[[nodiscard]] Handle handleOf(cref<Actor> actor_) const;

		[[nodiscard]] ptr<Actor> resolve(Handle handle_) const noexcept;

		/**
		 * Visits the archetype regions of all levels (detached actors excluded): one span per contiguous
		 * run of live slots per layout entry whose type is `Type_` or derives from it.
		 * shortcut: derivation is read from meta classes, so a `Type_` without its own meta class matches exactly;
		 * give such a type `InheritMeta` once derived queries on it are needed.
		 */
		template <typename Type_>
		void forEachSpan(FnRef<void(ComponentSpan<Type_>)> fn_) const {
			ptr<const MetaClass> meta = nullptr;
			if constexpr (detail::own_meta<Type_>::value) {
				meta = Type_::getStaticMetaClass();
			}
			forEachSpan(
				&detail::type_tag<Type_>,
				meta,
				[&fn_](std::byte* const base_, const u32 count_, const u32 stride_) {
					fn_(ComponentSpan<Type_> { base_, count_, stride_ });
				}
			);
		}

		#pragma endregion

	private:
		Housekeeping _housekeeping;

	public:
		[[nodiscard]] ref<Housekeeping> housekeeping() noexcept;

	private:
		RelocationBarrier _relocation;

	public:
		[[nodiscard]] ref<RelocationBarrier> relocation() noexcept;

		/**
		 * Builds a request that empties `chunk_` of the class's region in `space_`; for tests and compaction drivers.
		 */
		[[nodiscard]] MoveRequest debugMoveRequest(SpaceId space_, ptr<const ActorClass> class_, u32 chunk_);
	};
}