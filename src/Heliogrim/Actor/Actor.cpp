#include "Actor.hpp"

#include <algorithm>
#include <Engine.ACS/ActorModule.hpp>
#include <Engine.ACS/Registry.hpp>
#include <Engine.Asserts/Asserts.hpp>
#include <Engine.Async/Await/Promise.hpp>
#include <Engine.Common/Move.hpp>
#include <Engine.Common/Collection/Vector.hpp>
#include <Engine.Core/Engine.hpp>
#include <Engine.Core/Universe.hpp>
#include <Engine.Level/Level.hpp>
#include <Engine.Logging/Logger.hpp>
#include <Engine.Reflect/IsType.hpp>
#include <Engine.Scene/SceneBase.hpp>
#include <Engine.Serialization/Access/Structure.hpp>
#include <Engine.Serialization/Structure/StructScopedSlot.hpp>
#include <Engine.Serialization.Structures/Symbols.hpp>
#include <Engine.Serialization.Structures/Common/TypeId.hpp>

#include "ActorRef.hpp"
#include "../ActorInitializer.hpp"
#include "../IComponentRegisterContext.hpp"
#include "../Component/HierarchyComponent.hpp"
#include "../Core/Session.hpp"
#include "../Universe/Universe.hpp"
#include "Archetype/ActorClass.hpp"

/**/

using namespace hg;

/**/

namespace {
	[[nodiscard]] ref<engine::acs::Registry> actorRegistry() noexcept;

	[[nodiscard]] Vector<ActorRef<>> spawnInto(
		const ptr<const ActorClass> class_,
		const size_t count_,
		const FnRef<void(ref<Actor>, size_t)> init_,
		cref<Universe> universe_,
		Opt<ref<engine::core::Level>> level_
	);

	/**
	 * A missing key yields a null record slot that later dereferences, so every key hydrate reads is probed first.
	 */
	[[nodiscard]] bool acceptsRecord(
		const ptr<const ActorClass> class_,
		ref<const engine::serialization::StructScopedSlot> record_
	);
}

Actor::Actor(cref<ActorInitializer> initializer_) noexcept :
	_guid(initializer_._guid),
	_label(),
	_universe(None),
	_rootComponent(None),
	_archetype(initializer_._archetype),
	_components() {

	if (_archetype != nullptr) {
		// The root is entry 0, constructed in front of the actor before this constructor runs.
		auto* const slot = reinterpret_cast<std::byte*>(this) - _archetype->desc().actorOffset;
		_rootComponent = Some<ref<HierarchyComponent>>(
			*std::launder(reinterpret_cast<ptr<HierarchyComponent>>(slot + _archetype->componentOffsets().front()))
		);
	}
}

Actor::~Actor() noexcept = default;

ActorGuid Actor::guid() const noexcept {
	return _guid;
}

ref<const ActorLabel> Actor::label() const noexcept {
	return _label;
}

void Actor::setLabel(mref<ActorLabel> label_) {
	_label = ::hg::move(label_);
}

void Actor::unsafe_set_guid(cref<ActorGuid> guid_) {
	_guid = guid_;
}

Opt<ref<engine::core::Universe>> Actor::getUniverse() const noexcept {
	return _universe;
}

Opt<ref<engine::core::Level>> Actor::getLevel() const noexcept {
	return _level;
}

Opt<ref<HierarchyComponent>> Actor::getRootComponent() const noexcept {
	return _rootComponent;
}

void Actor::setOwnerLevel(mref<Opt<ref<engine::core::Level>>> owner_) {
	_level = ::hg::move(owner_);
}

ptr<const ActorClass> Actor::getArchetype() const noexcept {
	return _archetype;
}

void Actor::rejectSealed(const std::string_view message_) const {
	// shortcut: sealed archetypes reject runtime component changes until de-optimization exists (spec 11).
	if (_archetype != nullptr) [[unlikely]] {
		::hg::panic_details::PanicErrorSink {} << std::string_view { message_ };
		::hg::panic();
	}
}

void Actor::scheduleComponentRegister(const ptr<HierarchyComponent> component_) {
	_universe->getScene()->registerContext()->add(component_);
}

void Actor::scheduleComponentRemoval(mref<ptr<HierarchyComponent>> component_) {
	_universe->getScene()->registerContext()->remove(component_);
}

void Actor::scheduleComponentRelease(mref<ptr<LogicComponent>> component_) const {
	engine::Engine::getEngine()->getActors()->getRegistry()->releaseActorComponent(VolatileComponent<> { ::hg::move(component_) });
}

void Actor::addComponent(mref<VolatileComponent<HierarchyComponent>> component_, Opt<ref<const AddComponentOptions>> options_) {

	rejectSealed("Actor::addComponent: sealed archetype actor rejects runtime components");

	if (not options_.is_null() && options_->autoRegister && not _universe.is_null()) {
		scheduleComponentRegister(component_.get());
	}

	if (options_.is_null() || options_->parent.is_null()) {
		::hg::assertrt(component_->getParentComponent() == nullptr && _rootComponent.is_null());
		auto* const component = ::hg::move(component_).release();
		_rootComponent = Some<ref<HierarchyComponent>>(*component);
		_components.insert(component);
		return;
	}

	::hg::assertrt(component_->getParentComponent() == options_->parent.address());
	::hg::assertd(
		/* Check: Failed to ensure acknowledged parent component.*/
		_components.contains(component_->getParentComponent())
	);

	_components.insert(::hg::move(component_).release());
}

ComponentRange Actor::getComponents() const noexcept {
	if (_archetype == nullptr) {
		return ComponentRange::dynamic(_components);
	}

	const auto desc = _archetype->desc();
	return ComponentRange::archetype(
		const_cast<std::byte*>(reinterpret_cast<const std::byte*>(this)) - desc.actorOffset,
		_archetype->componentOffsets(),
		_archetype->startBitmap(),
		desc.actorOffset,
		_archetype->minAlign()
	);
}

void Actor::registerComponents(ref<engine::core::Universe> universe_, ref<IComponentRegisterContext> context_) {

	if (_universe != None && _universe.address() != std::addressof(universe_)) [[unlikely]] {
		IM_CORE_WARN("Tried to register actor components in a mismatching scene.");
		return;
	}

	_universe = Some<ref<engine::core::Universe>>(universe_);

	/**
	 * Guard for non rooted actor
	 */
	if (not _rootComponent.has_value()) {
		return;
	}

	/**
	 * Archetype entries are already ordered parent before child
	 */
	if (_archetype != nullptr) {
		for (auto* const entry : getComponents()) {
			context_.add(entry);
		}
		return;
	}

	/**
	 * Insert components ordered by hierarchy from parent to child (copy components)
	 */
	auto components { _components };

	auto iter { components.begin() };
	while (!components.empty()) {
		/**
		 *
		 */
		auto* const entry = *iter;
		const auto parent { entry->getParentComponent() };

		/**
		 *
		 */
		auto prevIter = components.find(parent);
		if (prevIter != components.end()) {
			iter = std::move(prevIter);
			continue;
		}

		/**
		 * Insert component
		 */
		context_.add(entry);

		/**
		 *
		 */
		auto nextIter { components.erase(iter) };
		iter = std::move(nextIter != components.end() ? nextIter : components.begin());
	}
}

void Actor::unregisterComponents(ref<IComponentRegisterContext> context_) {

	// TODO: Check whether we need to remove components in (hierarchical) order
	for (const auto& entry : getComponents()) {
		context_.remove(entry);
	}

	_universe = None;
}

template <>
void VolatileActor<Actor>::destroy(ptr<Actor> obj_) {

	::hg::assertd(obj_->getUniverse().is_null());
	auto registry = engine::Engine::getEngine()->getActors()->getRegistry();

	// Archetype components live in the actor's slot; they go with it, never to the per-type pools.
	if (obj_->getArchetype() != nullptr) {
		registry->destroyArchetypeActor(obj_);
		return;
	}

	for (auto* const component : obj_->getComponents()) {
		::hg::assertd(component->getTypeId().data);
		registry->releaseActorComponent(VolatileComponent<> { component });
	}
	registry->destroyActor(std::move(obj_));
}

bool hg::RegisterActorClass(ptr<const ActorClass> class_) noexcept {
	return actorRegistry().registerActorClass(class_);
}

Opt<ref<const ActorClass>> hg::FindActorClass(const type_id typeId_) noexcept {
	return actorRegistry().findActorClass(typeId_);
}

VolatileActor<> hg::CreateActor() {
	auto& registry = *engine::Engine::getEngine()->getActors()->getRegistry();
	return VolatileActor<> { registry.createActor(ActorInitializer { registry }) };
}

Future<VolatileActor<>> hg::CreateActor(async_t) {
	concurrent::promise<VolatileActor<>> p {
		[]() -> VolatileActor<> {
			auto& registry = *engine::Engine::getEngine()->getActors()->getRegistry();
			return VolatileActor<> { registry.createActor(ActorInitializer { registry }) };
		}
	};

	auto f { p.get() };
	p();

	return Future { std::move(f) };
}

VolatileActor<> hg::CreateActor(ptr<const MetaClass> class_) noexcept {
	auto& registry = *engine::Engine::getEngine()->getActors()->getRegistry();
	return VolatileActor<> { registry.createActor(class_, ActorInitializer { registry }) };
}

VolatileActor<> hg::CreateActor(ptr<const ActorClass> class_) noexcept {
	return VolatileActor<> { ::hg::move(actorRegistry().createActors(class_, 1uLL, engine::acs::detachedSpace).front()) };
}

Future<VolatileActor<>> hg::CreateActor(
	ptr<const ActorClass> class_,
	cref<SerializedActor> serialized_
) noexcept {
	concurrent::promise<VolatileActor<>> p {
		[class_, &serialized_]() -> VolatileActor<> {
			// Validate before allocating, so a rejected record never holds a slot.
			if (not acceptsRecord(class_, serialized_.record)) {
				return VolatileActor<> { ptr<Actor> { nullptr } };
			}
			auto actor = CreateActor(class_);
			engine::serialization::access::Structure<Actor>::hydrate(serialized_.record, *actor);
			return actor;
		}
	};

	auto f { p.get() };
	p();

	return Future { ::hg::move(f) };
}

Future<Opt<ActorRef<>>> hg::CloneActorInto(cref<Actor> actor_, cref<Universe> universe_) noexcept {
	::hg::todo_panic();
}

Future<Opt<ActorRef<>>> hg::SpawnActor(ptr<const ActorClass> class_, cref<Universe> universe_) noexcept {
	concurrent::promise<Opt<ActorRef<>>> p {
		[class_, &universe_]() -> Opt<ActorRef<>> {
			return Some(spawnInto(class_, 1uLL, [](ref<Actor>, size_t) {}, universe_, None).front());
		}
	};

	auto f { p.get() };
	p();

	return Future { ::hg::move(f) };
}

Future<Opt<ActorRef<>>> hg::SpawnActor(
	ptr<const ActorClass> class_,
	cref<SerializedActor> serialized_,
	cref<Universe> universe_
) noexcept {
	concurrent::promise<Opt<ActorRef<>>> p {
		[class_, &serialized_, &universe_]() -> Opt<ActorRef<>> {
			auto created = CreateActor(class_, serialized_).get();
			if (created == nullptr) {
				return None;
			}
			const auto level = universe_.unwrap()->getRootLevel();
			return ActorRef<>::of(level->addActor(::hg::move(created)));
		}
	};

	auto f { p.get() };
	p();

	return Future { ::hg::move(f) };
}

Future<Vector<ActorRef<>>> hg::SpawnActors(
	ptr<const ActorClass> class_,
	size_t count_,
	FnRef<void(ref<Actor>, size_t)> init_,
	cref<Universe> universe_,
	Opt<ref<engine::core::Level>> level_
) {
	concurrent::promise<Vector<ActorRef<>>> p {
		[=, &universe_]() -> Vector<ActorRef<>> {
			return spawnInto(class_, count_, init_, universe_, level_);
		}
	};

	auto f { p.get() };
	p();

	return Future { ::hg::move(f) };
}

bool hg::Destroy(mref<VolatileActor<>> actor_) noexcept {
	std::move(actor_).reset();
	return true;
}

/**/

namespace {
	[[nodiscard]] ref<engine::acs::Registry> actorRegistry() noexcept {
		return *engine::Engine::getEngine()->getActors()->getRegistry();
	}

	[[nodiscard]] Vector<ActorRef<>> spawnInto(
		const ptr<const ActorClass> class_,
		const size_t count_,
		const FnRef<void(ref<Actor>, size_t)> init_,
		cref<Universe> universe_,
		Opt<ref<engine::core::Level>> level_
	) {
		const auto root = universe_.unwrap()->getRootLevel();
		auto* const level = level_.has_value() ? std::addressof(level_.value()) : root.get();

		auto actors = actorRegistry().createActors(class_, count_, level->space());
		for (size_t index = 0uLL; index < actors.size(); ++index) {
			init_(*actors[index], index);
		}
		// Already in the level's space, so addActor mounts without relocating.
		Vector<ActorRef<>> refs {};
		refs.reserve(actors.size());
		for (auto& actor : actors) {
			auto mounted = ActorRef<>::of(level->addActor(VolatileActor<> { ::hg::move(actor) }));
			// Invariant: SpawnActors only instantiates archetype classes, so every mounted actor has a handle.
			::hg::assertrt([&mounted] { return mounted.has_value(); });
			refs.emplace_back(mounted.value());
		}
		return refs;
	}

	/**
	 * A missing key yields a null record slot that later dereferences, so every key hydrate reads is probed first.
	 */
	[[nodiscard]] bool acceptsRecord(
		const ptr<const ActorClass> class_,
		ref<const engine::serialization::StructScopedSlot> record_
	) {
		using engine::serialization::Symbols;
		return record_.hasRecordSlot(Symbols::Type)
			&& record_.hasRecordSlot(Symbols::Guid)
			&& record_.hasRecordSlot(std::string_view { "label" })
			&& record_.hasRecordSlot(Symbols::Components)
			&& engine::serialization::get_typeId_slot(record_) == class_->metaClass()->typeId();
	}
}
