#include "Registry.hpp"

#include <Heliogrim/ActorInitializer.hpp>
#include <Heliogrim/Actor/Actor.hpp>

using namespace hg::engine::acs;
using namespace hg;

Registry::Registry() :
	_pools(),
	_actorPools(),
	_housekeeping(*this) {}

Registry::~Registry() {
	_housekeeping.commit();
}

void Registry::removeActorComponent(ref<const ComponentGuid> guid_, ref<const ComponentTypeId> typeId_) {
	auto* pool = _pools.at(typeId_);
	pool->erase(guid_);
}

void Registry::releaseActorComponent(mref<VolatileComponent<>> component_) {
	_housekeeping.storePendingRelease(::hg::move(component_));
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

	::hg::assertd(actor->getMetaClass<>() == actorClass_);

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

ref<Housekeeping> Registry::housekeeping() noexcept {
	return _housekeeping;
}
