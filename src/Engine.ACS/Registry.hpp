#pragma once

#include <Engine.Common/Make.hpp>
#include <Engine.Common/Collection/DenseMap.hpp>
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

namespace hg {
	/**
	 * Forward Declaration
	 */
	class Actor;
	class ActorInitializer;

	using ActorClass = ::hg::MetaClass;
}

namespace hg::engine::acs {
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
			const ptr<const ActorClass> actorClass_,
			cref<ActorInitializer> initializer_
		);

		[[nodiscard]] ptr<Actor> createActor(
			const ptr<const ActorClass> actorClass_,
			cref<ActorInitializer> initializer_,
			std::nothrow_t
		) noexcept;

	public:
		[[nodiscard]] ptr<Actor> createActor(cref<ActorInitializer> initializer_) noexcept;

		void destroyActor(mref<ptr<Actor>> actor_);

	private:
		void hackActorInit(cref<ActorInitializer> initializer_, cref<ActorGuid> guid_) const noexcept;

		#pragma endregion

	private:
		Housekeeping _housekeeping;

	public:
		[[nodiscard]] ref<Housekeeping> housekeeping() noexcept;
	};
}