#include "Registry.hpp"

#include <Engine.Common/Wrapper.hpp>

#include <algorithm>
#include <bit>
#include <new>
#include <Engine.ACS/Handle/Epoch.hpp>
#include <Engine.Asserts/Asserts.hpp>
#include <Heliogrim/ActorInitializer.hpp>
#include <Heliogrim/Actor/Actor.hpp>
#include <Heliogrim/Actor/Archetype/ActorClass.hpp>

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

ptr<Actor> Registry::createActor(const ptr<const MetaClass> actorClass_, cref<ActorInitializer> initializer_) {

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
	const ptr<const MetaClass> actorClass_,
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

/* Archetype Clusters */

namespace {
	[[nodiscard]] std::byte* slotOf(cref<Actor> actor_, cref<ActorClass> class_) noexcept {
		return const_cast<std::byte*>(reinterpret_cast<const std::byte*>(&actor_)) - class_.desc().actorOffset;
	}

	[[nodiscard]] ptr<Actor> actorAt(std::byte* const slot_, cref<ActorClass> class_) noexcept {
		// See the shortcut in ArchetypeDefinition: the Actor base sits at offset 0 of the archetype type.
		return std::launder(reinterpret_cast<ptr<Actor>>(slot_ + class_.desc().actorOffset));
	}
}

bool Registry::registerActorClass(const ptr<const ActorClass> class_) noexcept {
	if (class_ == nullptr) {
		return false;
	}
	const auto [where, inserted] = _classes.try_emplace(class_->metaClass()->typeId(), class_);
	return inserted || where->second == class_;
}

Opt<ref<const ActorClass>> Registry::findActorClass(const type_id typeId_) const noexcept {
	const auto where = _classes.find(typeId_);
	if (where == _classes.end()) {
		return None;
	}
	return Some(*where->second);
}

SpaceId Registry::acquireSpace() {
	const auto space = engine::acs::acquireSpace();
	_spaces.try_emplace(space, make_uptr<Space>(space));
	return space;
}

void Registry::releaseSpace(const SpaceId space_) {
	// The detached space lives as long as the registry.
	if (space_ == detachedSpace) {
		return;
	}
	const auto where = _spaces.find(space_);
	if (where != _spaces.end()) {
		// Pending moves would touch the freed regions; regions return their chunks, then the handle table goes.
		for (const auto& [actorClass, region] : where->second->regions) {
			_relocation.discard(region.get());
		}
		where->second->regions.clear();
		where->second->handles.releaseAll();
		_spaces.erase(where);
		Epoch::bump();
	}
	engine::acs::releaseSpace(space_);
}

bool Registry::hasSpace(const SpaceId space_) const noexcept {
	return _spaces.contains(space_);
}

ptr<const ClusterRegion> Registry::region(const SpaceId space_, const ptr<const ActorClass> class_) const noexcept {
	const auto space = _spaces.find(space_);
	if (space == _spaces.end()) {
		return nullptr;
	}
	const auto region = space->second->regions.find(class_);
	return region != space->second->regions.end() ? region->second.get() : nullptr;
}

ref<ClusterRegion> Registry::regionFor(const SpaceId space_, const ptr<const ActorClass> class_) {
	auto& space = _spaces[space_];
	if (space == nullptr) {
		space = make_uptr<Space>(space_);
	}
	auto& region = space->regions[class_];
	if (region == nullptr) {
		region = make_uptr<ClusterRegion>(class_->desc(), space->handles);
	}
	return *region;
}

Registry::Location Registry::locate(cref<Actor> actor_) const {
	const auto* const actorClass = actor_.getArchetype();
	if (actorClass == nullptr) {
		return Location { nullptr, nullptr, nullptr, Handle {} };
	}

	auto* const slot = slotOf(actor_, *actorClass);
	// shortcut: scans every space for the actor's region, add a slot-to-space index if spaces grow past a few dozen
	for (const auto& [id, space] : _spaces) {
		const auto region = space->regions.find(actorClass);
		if (region == space->regions.end()) {
			continue;
		}
		const auto handle = region->second->handleOf(slot);
		if (handle != Handle {}) {
			return Location { space.get(), region->second.get(), slot, handle };
		}
	}
	return Location { nullptr, nullptr, slot, Handle {} };
}

Vector<ptr<Actor>> Registry::createActors(const ptr<const ActorClass> class_, const size_t count_, const SpaceId space_) {
	auto& region = regionFor(space_, class_);
	const auto stride = class_->desc().stride;

	Vector<Handle> handles(count_);
	const auto slots = region.allocate(count_, handles);

	Vector<ptr<Actor>> result {};
	result.reserve(count_);
	for (size_t first = 0uLL; first < count_;) {
		auto last = first + 1uLL;
		while (last < count_ && slots[last] == slots[last - 1uLL] + stride) {
			++last;
		}
		class_->ops().construct(slots[first], static_cast<u32>(last - first), stride, ArchetypeInit { *this });
		for (; first < last; ++first) {
			result.emplace_back(actorAt(slots[first], *class_));
		}
	}
	return result;
}

ptr<Actor> Registry::moveActor(const ptr<Actor> actor_, const SpaceId space_) {
	const auto from = locate(*actor_);
	::hg::assertrt(from.region != nullptr);
	if (from.space->handles.space() == space_) {
		return actor_;
	}

	const auto* const actorClass = actor_->getArchetype();
	auto& target = regionFor(space_, actorClass);
	Handle handle {};
	auto* const slot = target.allocate(handle);

	actorClass->ops().relocate(from.slot, slot, 1u, actorClass->desc().stride);
	// Retired, not freed: a pointer to the old address taken this frame must not alias a new occupant.
	from.region->retire(from.slot);
	// The old handle is gone now; cached refs must not serve the moved-from slot until housekeep.
	Epoch::bump();
	return actorAt(slot, *actorClass);
}

void Registry::destroyArchetypeActor(const ptr<Actor> actor_) {
	const auto at = locate(*actor_);
	::hg::assertrt(at.region != nullptr);

	const auto* const actorClass = actor_->getArchetype();
	actorClass->ops().destroy(at.slot, 1u, actorClass->desc().stride);
	at.region->retire(at.slot);
	Epoch::bump();
}

Handle Registry::handleOf(cref<Actor> actor_) const {
	return locate(actor_).handle;
}

ptr<Actor> Registry::resolve(const Handle handle_) const noexcept {
	const auto space = _spaces.find(handle_.space());
	if (space == _spaces.end()) {
		return nullptr;
	}
	return static_cast<ptr<Actor>>(space->second->handles.resolve(handle_));
}

std::byte* Registry::resolveComponent(const Handle handle_, const std::ptrdiff_t offset_) const noexcept {
	auto* const actor = resolve(handle_);
	const auto* const actorClass = actor != nullptr ? actor->getArchetype() : nullptr;
	if (actorClass == nullptr) {
		return nullptr;
	}
	// Component offsets are slot-relative, ref offsets actor-relative.
	const auto fromSlot = offset_ + static_cast<std::ptrdiff_t>(actorClass->desc().actorOffset);
	if (fromSlot < 0 || not std::ranges::contains(actorClass->componentOffsets(), static_cast<uintptr_t>(fromSlot))) {
		return nullptr;
	}
	return reinterpret_cast<std::byte*>(actor) + offset_;
}

void Registry::forEachSpan(
	const ptr<const void> tag_,
	const ptr<const MetaClass> meta_,
	const FnRef<void(std::byte*, u32, u32)> fn_
) const {
	Vector<u32> offsets {};
	for (const auto& [id, space] : _spaces) {
		if (id == detachedSpace) {
			continue;
		}

		for (const auto& [actorClass, region] : space->regions) {
			offsets.clear();
			const auto types = actorClass->entryTypes();
			for (size_t entry = 0uLL; entry < types.size(); ++entry) {
				if (types[entry].tag == tag_ || (meta_ != nullptr && types[entry].meta->is(meta_->typeId()))) {
					// See the shortcut in ArchetypeDefinition: a base subobject sits at offset 0 of its component.
					offsets.emplace_back(actorClass->entries()[entry].offset);
				}
			}
			if (offsets.empty()) {
				continue;
			}

			region->forEachChunk(
				[&](std::byte* const base_, const u32 count_, const u32 stride_, const u64* const live_) {
					const auto isLive = [live_](const u32 index_) {
						return (live_[index_ / 64u] >> (index_ % 64u)) & 1uLL;
					};
					for (u32 first = 0u; first < count_;) {
						if (not isLive(first)) {
							++first;
							continue;
						}
						auto last = first + 1u;
						while (last < count_ && isLive(last)) {
							++last;
						}
						for (const auto offset : offsets) {
							fn_(base_ + static_cast<size_t>(first) * stride_ + offset, last - first, stride_);
						}
						first = last;
					}
				}
			);
		}
	}
}

bool Registry::reclaimSlots() {
	bool reclaimed = false;
	for (const auto& [id, space] : _spaces) {
		for (const auto& [actorClass, region] : space->regions) {
			reclaimed = region->reclaim() || reclaimed;
		}
	}
	return reclaimed;
}
