#include "Registry.hpp"

#include <Heliogrim/Actor/Actor.hpp>

#include "Heliogrim/ActorInitializer.hpp"

using namespace hg::engine::acs;
using namespace hg;

Registry::~Registry() = default;

void Registry::releaseActorComponent(cref<ActorGuid> guid_, cref<type_id> typeId_) {
	auto* entry { _pools.at(typeId_) };
	auto* pool { static_cast<ptr<PoolWrapperBase>>(entry) };

	pool->erase(guid_);
}

ptr<Actor> Registry::createActor(const ptr<const ActorClass> actorClass_, cref<ActorInitializer> initializer_) {

	const auto guid = generate_actor_guid();
	hackActorInit(initializer_, guid);

	/**/

	auto* const pool { _actorPools.at(actorClass_->typeId()) };
	pool->insert(guid, initializer_);

	/**/

	auto* actor { static_cast<ptr<Actor>>(pool->get(guid)) };
	actor->unsafe_set_guid(guid);

	// Warning: Error prone behaviour
	// Warning: Just a hotfix - ClassMetaBase has class pointer and vt-pointer
	static_assert(sizeof(ClassMetaBase) == sizeof(void*) + sizeof(ptr<MetaClass>));
	const auto** classLocator = reinterpret_cast<ptr<ptr<const ActorClass>>>(
		std::addressof(*static_cast<ptr<ClassMetaBase>>(actor)) + ptrdiff_t {
			/*offsetof(ClassMetaBase, _meta)*/8LL
		}
	);
	*classLocator = actorClass_;

	/**/

	return actor;
}

ptr<Actor> Registry::createActor(
	const ptr<const ActorClass> actorClass_,
	cref<ActorInitializer> initializer_,
	std::nothrow_t
) noexcept {

	const auto guid = generate_actor_guid();
	hackActorInit(initializer_, guid);

	const auto poolIter { _actorPools.find(actorClass_->typeId()) };
	if (poolIter == _actorPools.end()) {
		return nullptr;
	}

	/**/

	auto* const pool { _actorPools.at(actorClass_->typeId()) };
	pool->insert(guid, initializer_);

	/**/

	auto* actor { static_cast<ptr<Actor>>(pool->get(guid)) };
	if (actor == nullptr) {
		return nullptr;
	}

	/**/

	actor->unsafe_set_guid(guid);

	// Warning: Error prone behaviour
	// Warning: Just a hotfix - ClassMetaBase has class pointer and vt-pointer
	static_assert(sizeof(ClassMetaBase) == sizeof(void*) + sizeof(ptr<MetaClass>));
	const auto** classLocator = reinterpret_cast<ptr<ptr<const ActorClass>>>(
		std::addressof(*static_cast<ptr<ClassMetaBase>>(actor)) + ptrdiff_t {
			/*offsetof(ClassMetaBase, _meta)*/8LL
		}
	);
	*classLocator = actorClass_;

	/**/

	return actor;
}

ptr<Actor> Registry::createActor(cref<ActorInitializer> initializer_) noexcept {

	const auto guid = generate_actor_guid();
	hackActorInit(initializer_, guid);

	/**/

	auto* const pool = getOrCreateActorPool<Actor>();
	pool->emplace(guid, initializer_);

	/**/

	auto* actor = pool->get(guid);
	if (actor == nullptr) {
		return nullptr;
	}

	/**/

	actor->unsafe_set_guid(guid);

	// Warning: Error prone behaviour
	// Warning: Just a hotfix - ClassMetaBase has class pointer and vt-pointer
	static_assert(sizeof(ClassMetaBase) == sizeof(void*) + sizeof(ptr<MetaClass>));
	const auto** classLocator = reinterpret_cast<ptr<ptr<const ActorClass>>>(
		std::addressof(*static_cast<ptr<ClassMetaBase>>(actor)) + ptrdiff_t {
			/*offsetof(ClassMetaBase, _meta)*/8LL
		}
	);
	*classLocator = TypedMetaClass<Actor>::get();

	/**/

	return actor;
}

void Registry::destroyActor(mref<ptr<Actor>> actor_) {
	auto* const pool { _actorPools.at(actor_->getMetaClass()->typeId()) };
	pool->erase(actor_->guid());
}

void Registry::hackActorInit(cref<ActorInitializer> initializer_, cref<ActorGuid> guid_) const noexcept {
	// Warning: Hotfix - Hack
	const_cast<ref<ActorInitializer>>(initializer_)._guid = guid_;
}
