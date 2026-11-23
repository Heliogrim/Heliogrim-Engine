#include "pch.h"

/**/

#include <Engine.ACS/Registry.hpp>
#include <Engine.ACS/Traits.hpp>
#include <Engine.Diagnostics/Render.hpp>
#include <Engine.Diagnostics/Sink.hpp>
#include <Engine.Level/Level.hpp>
#include <Engine.Resource.Archive/ArchiveStreamMode.hpp>
#include <Engine.Resource.Archive/BufferArchive.hpp>
#include <Engine.Serialization/SerializationModule.hpp>
#include <Engine.Serialization/Access/Structure.hpp>
#include <Engine.Serialization/Archive/StructuredArchive.hpp>
#include <Engine.Serialization/Load/LoadContext.hpp>
#include <Engine.Serialization/Load/RootKind.hpp>
#include <Engine.Serialization.Structures/Common/Guid.hpp>
#include <Engine.Serialization/Scheme/ActorScheme.hpp>
#include <Engine.Serialization/Structure/RootScopedSlot.hpp>
#include <Engine.Serialization/Structure/SeqScopedSlot.hpp>
#include <Heliogrim/Actor/Actor.hpp>
#include <Heliogrim/Actor/Archetype/ArchetypeFnSet.hpp>
#include <Heliogrim/Component/LogicComponent.hpp>

#include "SpawnEngine.hpp"
#include "TestArchetypes.hpp"

using namespace hg::engine::serialization;
using namespace hg::engine::resource;
using namespace hg::engine::core;
using namespace hg;

namespace LoadBoundaryModule {
	constexpr u64 testActorType = 0x7E57'0000'0000'0011uLL;
	constexpr u64 testComponentType = 0x7E57'0000'0000'0012uLL;
	constexpr u64 plainComponentType = 0x7E57'0000'0000'0013uLL;

	inline u32 hydratedValue = 0u;

	struct TestAccess {
		template <typename = void>
		static ptr<Actor> instantiate(nmpt<engine::acs::Registry> allocator_, cref<ActorGuid> guid_) {
			return ActorScheme<Actor>::instantiate<void>(allocator_, guid_);
		}

		static void hydrate(cref<StructScopedSlot> slot_, ref<Actor>) {
			if (slot_.hasRecordSlot("value")) {
				slot_.getSlot<u32>("value") >> hydratedValue;
			}
		}
	};

	/* Non-hierarchy component; ActorScheme cannot attach it. */
	struct PlainComponent : LogicComponent {
		PlainComponent() noexcept :
			LogicComponent(ComponentGuid {}, ComponentTypeId {}) {}
	};

	Result<void, MigrateError> addValue(ref<const StructScopedSlot> old_, ref<StructScopedSlot> new_) {
		forwardEntries(old_, new_, {});
		new_.insertSlot<u32>("value") << 7u;
		return {};
	}

	Result<void, MigrateError> scaleValue(ref<const StructScopedSlot> old_, ref<StructScopedSlot> new_) {
		forwardEntries(old_, new_, { "value" });
		u32 value = 0u;
		old_.getSlot<u32>("value") >> value;
		new_.insertSlot<u32>("value") << value * 10u + 1u;
		return {};
	}

	inline u8 guidSeed = 0x80;

	[[nodiscard]] type_id typeOf(const u64 data_) {
		type_id id {};
		id.data = data_;
		return id;
	}

	struct ComponentSpec {
		u64 type = testComponentType;
		s8 parent = -1;
		bool withType = true;
		u16 rev = 0;
	};

	[[nodiscard]] bool hasCode(cref<engine::diag::CollectingSink> sink_, StringView key_) {
		for (const auto& item : sink_.items()) {
			if (item.code() != nullptr && item.code()->key == key_) {
				return true;
			}
		}
		return false;
	}

	class LoadBoundary : public ::testing::Test {
	protected:
		uptr<HeliogrimApi::test_engine::SpawnEngine> _engine;
		BufferArchive _archive {};
		LoadContext _ctx { 1uLL << 20 };
		engine::diag::CollectingSink _sink {};

		void SetUp() override {
			_engine = make_uptr<HeliogrimApi::test_engine::SpawnEngine>();

			_engine->_serialization->addActorScheme(
				typeOf(testActorType),
				ActorFnSet {
					.serialize = [](cref<Actor>, mref<StructScopedSlot>) {},
					.deserialize = [](cref<RecordScopedSlot> slot_, nmpt<engine::acs::Registry> allocator_) -> ptr<Actor> {
						return ActorScheme<Actor>::deserialize<TestAccess>(slot_, allocator_);
					}
				}
			);

			_engine->_serialization->addComponentScheme(
				typeOf(testComponentType),
				ComponentFnSet {
					.serialize = [](cref<LogicComponent>, mref<StructScopedSlot>) {},
					.deserialize = [](cref<RecordScopedSlot>, nmpt<engine::acs::Registry>, ref<Actor>, nmpt<HierarchyComponent>) {
						return VolatileComponent<> { static_cast<ptr<LogicComponent>>(nullptr) };
					}
				}
			);
			registerExtraSchemes();
		}

		void registerExtraSchemes() {
			_engine->_serialization->addComponentScheme(
				typeOf(plainComponentType),
				ComponentFnSet {
					.serialize = [](cref<LogicComponent>, mref<StructScopedSlot>) {},
					.deserialize = [](cref<RecordScopedSlot>, nmpt<engine::acs::Registry>, ref<Actor>, nmpt<HierarchyComponent>) {
						return VolatileComponent<> { new PlainComponent {}, [](ptr<LogicComponent> obj_) { delete obj_; } };
					}
				}
			);

			_engine->_serialization->addActorScheme(
				HeliogrimApi::TestTank::typeId,
				MakeArchetypeFnSet(&HeliogrimApi::TestTank::actorClass())
			);
			hydratedValue = 0u;
		}

		void TearDown() override {
			_engine.reset();
		}

		[[nodiscard]] nmpt<engine::acs::Registry> registry() const {
			return _engine->getActors()->getRegistry();
		}

		void writeLevel(const u16 actorRev_, std::span<const ComponentSpec> components_, const u64 actorType_ = testActorType) {
			auto structured = StructuredArchive { _archive };
			auto root = structured.insertRootSlot().intoStruct();
			access::Structure<math::Bounding>::serialize(math::Bounding {}, root.insertStructSlot("bounding"));
			auto actors = root.insertRecordSlot(Symbols::Components).intoSeq();
			auto actor = actors.addRecordSlot().intoStruct();
			actor.insertSlot<u64>(Symbols::Type) << actorType_;
			if (actorRev_ > 0) {
				actor.insertSlot<u16>("@rev") << actorRev_;
			}
			actor.insertSlot<string>(Symbols::Guid) << string(16uLL, static_cast<char>(++guidSeed));
			auto seq = actor.insertRecordSlot(Symbols::Components).intoSeq();
			for (const auto& spec : components_) {
				auto component = seq.addRecordSlot().intoStruct();
				if (spec.withType) {
					component.insertSlot<u64>(Symbols::Type) << spec.type;
				}
				if (spec.rev > 0) {
					component.insertSlot<u16>("@rev") << spec.rev;
				}
				component.insertSlot<s8>(Symbols::Parent) << spec.parent;
			}
		}

		[[nodiscard]] Arci<Level> hydrateLevel() {
			_archive.seek(0);
			const auto root = RootScopedSlot { _archive, ScopedSlotStateFlag::eClean, &_ctx };
			auto level = make_root_like_level();
			access::Structure<Level>::hydrate(root.asStruct(), *level);
			if (auto result = _ctx.finish(_sink, "level"); not result.has_value()) {
				for (auto& diagnostic : result.error()) {
					_sink.emit(::hg::move(diagnostic));
				}
			}
			return level;
		}
	};

	TEST_F(LoadBoundary, NewerRevisionActorSkippedAndRefused) {
		writeLevel(3, {});
		const auto level = hydrateLevel();

		EXPECT_TRUE(level->getActors().empty());
		EXPECT_TRUE(level->carriesUnknownData());
		EXPECT_TRUE(hasCode(_sink, "revision-newer"));
		EXPECT_FALSE(_sink.hasErrors());

		const auto saved = checkSaveAllowed(*level);
		ASSERT_FALSE(saved.has_value());
		EXPECT_EQ(String { saved.error().front().code()->key }, "save-refused");
	}

	TEST_F(LoadBoundary, NewerRevisionComponentMarksActor) {
		const ComponentSpec components[] { { .rev = 2 } };
		writeLevel(0, components);
		const auto level = hydrateLevel();

		ASSERT_EQ(level->getActors().size(), 1uLL);
		EXPECT_TRUE(level->getActors().front()->carriesUnknownData());
		EXPECT_TRUE(hasCode(_sink, "revision-newer"));
		EXPECT_FALSE(checkSaveAllowed(*level).has_value());
	}

	TEST_F(LoadBoundary, ParentIndexOutOfRange) {
		const ComponentSpec components[] { { .parent = 5 } };
		writeLevel(0, components);
		const auto level = hydrateLevel();

		EXPECT_TRUE(level->getActors().empty());
		EXPECT_TRUE(hasCode(_sink, "invalid-value"));
		EXPECT_TRUE(_sink.hasErrors());
	}

	TEST_F(LoadBoundary, MissingComponentTypeSkipsActor) {
		const ComponentSpec components[] { { .withType = false } };
		writeLevel(0, components);
		const auto level = hydrateLevel();

		EXPECT_TRUE(level->getActors().empty());
		EXPECT_TRUE(level->carriesUnknownData());
		EXPECT_TRUE(hasCode(_sink, "unknown-type"));
		EXPECT_FALSE(_sink.hasErrors());
	}

	TEST_F(LoadBoundary, RootActorComponentWithoutTypeRejected) {
		const auto guid = generate_actor_guid();
		{
			auto structured = StructuredArchive { _archive };
			auto actor = structured.insertRootSlot().intoStruct();
			actor.insertSlot<u64>(Symbols::Type) << testActorType;
			actor.insertSlot<string>(Symbols::Guid) << string(reinterpret_cast<const char*>(&guid), sizeof(guid));
			auto seq = actor.insertRecordSlot(Symbols::Components).intoSeq();
			seq.addRecordSlot().intoStruct().insertSlot<s8>(Symbols::Parent) << s8 { -1 };
		}
		_archive.seek(0);
		{
			const auto root = RootScopedSlot { _archive, ScopedSlotStateFlag::eClean, &_ctx };
			EXPECT_EQ((ActorScheme<Actor>::deserialize<TestAccess>(root, registry())), nullptr);
		}
		EXPECT_FALSE(registry()->contains(guid));
		ASSERT_TRUE(_ctx.failed());
		EXPECT_EQ(_ctx.firstError()->kind, ReadError::Kind::eUnknownType);
	}

	TEST(LoadBoundaryNoEngine, ArchetypeRecordWithoutSerializationRefused) {
		if (engine::Engine::getEngine() != nullptr) {
			GTEST_SKIP() << "needs a process without a live engine";
		}
		BufferArchive archive {};
		{
			auto structured = StructuredArchive { archive };
			structured.insertRootSlot().intoStruct().insertSlot<u32>("hp") << 7u;
		}
		archive.seek(0);
		LoadContext ctx { static_cast<u64>(archive.totalSize()) };
		{
			const auto root = RootScopedSlot { archive, ScopedSlotStateFlag::eClean, &ctx };
			EXPECT_FALSE(migrateArchetypeRecord(HeliogrimApi::TestTank::typeId, root.asStruct()).has_value());
		}
		ASSERT_TRUE(ctx.failed());
		EXPECT_EQ(ctx.firstError()->kind, ReadError::Kind::eMigrationFailed);

		// Without a load context the record hydrates as written.
		const auto contextless = RootScopedSlot { archive, ScopedSlotStateFlag::eClean };
		EXPECT_TRUE(migrateArchetypeRecord(HeliogrimApi::TestTank::typeId, contextless.asStruct()).has_value());
	}

	TEST_F(LoadBoundary, FailedComponentMarksActor) {
		const ComponentSpec components[] { { .type = testComponentType } };
		writeLevel(0, components);
		const auto level = hydrateLevel();

		ASSERT_EQ(level->getActors().size(), 1uLL);
		EXPECT_TRUE(level->getActors().front()->carriesUnknownData());
		EXPECT_FALSE(hasCode(_sink, "unknown-field"));
		EXPECT_FALSE(checkSaveAllowed(*level).has_value());
	}

	TEST_F(LoadBoundary, ChildOfFailedComponentMarksActor) {
		const ComponentSpec components[] { { .type = testComponentType }, { .type = testComponentType, .parent = 0 } };
		writeLevel(0, components);
		const auto level = hydrateLevel();

		ASSERT_EQ(level->getActors().size(), 1uLL);
		EXPECT_TRUE(level->getActors().front()->carriesUnknownData());
	}

	TEST_F(LoadBoundary, RegistryContainsCreatedActorOnly) {
		const auto guid = generate_actor_guid();
		const auto other = generate_actor_guid();
		const auto* const actor = ActorScheme<Actor>::instantiate<void>(registry(), guid);

		ASSERT_NE(actor, nullptr);
		EXPECT_TRUE(registry()->contains(guid));
		EXPECT_FALSE(registry()->contains(other));
	}

	TEST_F(LoadBoundary, MigrationStepLandsInHydrate) {
		const Migration steps[] { { typeOf(testActorType), 0, &addValue } };
		_engine->_serialization->migrations().add(steps);
		writeLevel(0, {});
		const auto level = hydrateLevel();

		ASSERT_EQ(level->getActors().size(), 1uLL);
		EXPECT_EQ(hydratedValue, 7u);
		EXPECT_FALSE(level->getActors().front()->carriesUnknownData());
		EXPECT_FALSE(_sink.hasErrors());
	}

	TEST_F(LoadBoundary, MigrationChainLandsInHydrate) {
		const Migration steps[] { { typeOf(testActorType), 0, &addValue }, { typeOf(testActorType), 1, &scaleValue } };
		_engine->_serialization->migrations().add(steps);
		writeLevel(0, {});
		const auto level = hydrateLevel();

		ASSERT_EQ(level->getActors().size(), 1uLL);
		EXPECT_EQ(hydratedValue, 71u);
		EXPECT_FALSE(level->getActors().front()->carriesUnknownData());
		EXPECT_FALSE(_sink.hasErrors());
	}

	TEST_F(LoadBoundary, NonHierarchyComponentMarksActor) {
		const ComponentSpec components[] { { .type = plainComponentType } };
		writeLevel(0, components);
		const auto level = hydrateLevel();

		ASSERT_EQ(level->getActors().size(), 1uLL);
		EXPECT_TRUE(level->getActors().front()->carriesUnknownData());
		EXPECT_FALSE(checkSaveAllowed(*level).has_value());
	}

	TEST_F(LoadBoundary, NewerRevisionArchetypeActorSkippedAndRefused) {
		writeLevel(3, {}, HeliogrimApi::TestTank::typeId.data);
		const auto level = hydrateLevel();

		EXPECT_TRUE(level->getActors().empty());
		EXPECT_TRUE(level->carriesUnknownData());
		EXPECT_TRUE(hasCode(_sink, "revision-newer"));
		EXPECT_FALSE(_sink.hasErrors());

		const auto saved = checkSaveAllowed(*level);
		ASSERT_FALSE(saved.has_value());
		EXPECT_EQ(String { saved.error().front().code()->key }, "save-refused");
	}

	/* One plain actor per guid; `lastParent_ >= 0` gives the last actor a component with that (invalid) parent. */
	/* A set `componentGuid_` gives every actor one component carrying that guid. */
	void writeActors(
		ref<BufferArchive> archive_,
		std::span<const ActorGuid> guids_,
		const s8 lastParent_ = -1,
		const Opt<ComponentGuid> componentGuid_ = None
	) {
		auto structured = StructuredArchive { archive_ };
		auto root = structured.insertRootSlot().intoStruct();
		access::Structure<math::Bounding>::serialize(math::Bounding {}, root.insertStructSlot("bounding"));
		auto actors = root.insertRecordSlot(Symbols::Components).intoSeq();
		for (size_t idx = 0uLL; idx < guids_.size(); ++idx) {
			auto actor = actors.addRecordSlot().intoStruct();
			actor.insertSlot<u64>(Symbols::Type) << testActorType;
			insert_guid_slot(actor, guids_[idx]);
			auto seq = actor.insertRecordSlot(Symbols::Components).intoSeq();
			if (lastParent_ >= 0 && idx + 1uLL == guids_.size()) {
				auto component = seq.addRecordSlot().intoStruct();
				component.insertSlot<u64>(Symbols::Type) << testComponentType;
				component.insertSlot<s8>(Symbols::Parent) << lastParent_;
			}
			if (componentGuid_.has_value()) {
				auto component = seq.addRecordSlot().intoStruct();
				component.insertSlot<u64>(Symbols::Type) << testComponentType;
				insert_guid_slot(component, *componentGuid_);
			}
		}
	}

	TEST_F(LoadBoundary, FailureOnLastActorLeavesRegistryClean) {
		const ActorGuid guids[] { generate_actor_guid(), generate_actor_guid() };
		writeActors(_archive, guids, 5);
		const auto level = hydrateLevel();

		EXPECT_TRUE(level->getActors().empty());
		EXPECT_FALSE(registry()->contains(guids[0]));
		EXPECT_FALSE(registry()->contains(guids[1]));
		EXPECT_TRUE(_sink.hasErrors());
	}

	TEST_F(LoadBoundary, DuplicateGuidAgainstLiveActorRejected) {
		const auto guid = generate_actor_guid();
		const auto live = ActorScheme<Actor>::instantiate<void>(registry(), guid);
		ASSERT_NE(live, nullptr);

		const ActorGuid guids[] { guid };
		writeActors(_archive, guids);
		const auto level = hydrateLevel();

		EXPECT_TRUE(level->getActors().empty());
		EXPECT_TRUE(hasCode(_sink, "duplicate-identity"));
		EXPECT_TRUE(_sink.hasErrors());
		EXPECT_TRUE(registry()->contains(guid));
		EXPECT_EQ(live->guid(), guid);
	}

	TEST_F(LoadBoundary, DuplicateGuidWithinLevelRejected) {
		const auto guid = generate_actor_guid();
		const ActorGuid guids[] { guid, guid };
		writeActors(_archive, guids);
		const auto level = hydrateLevel();

		EXPECT_TRUE(level->getActors().empty());
		EXPECT_TRUE(hasCode(_sink, "duplicate-identity"));
		EXPECT_FALSE(registry()->contains(guid));
	}

	TEST_F(LoadBoundary, DuplicateComponentGuidWithinLevelRejected) {
		const ActorGuid guids[] { generate_actor_guid(), generate_actor_guid() };
		writeActors(_archive, guids, -1, ComponentGuid { generate_component_guid() });
		const auto level = hydrateLevel();

		EXPECT_TRUE(level->getActors().empty());
		EXPECT_TRUE(hasCode(_sink, "duplicate-identity"));
		EXPECT_FALSE(registry()->contains(guids[0]));
		EXPECT_FALSE(registry()->contains(guids[1]));
	}

	TEST_F(LoadBoundary, DuplicateComponentGuidAgainstLiveRejected) {
		const auto owner = ActorScheme<Actor>::instantiate<void>(registry(), generate_actor_guid());
		ASSERT_NE(owner, nullptr);
		const auto componentGuid = ComponentGuid { generate_component_guid() };
		auto live = registry()->acquireActorComponent<HeliogrimApi::BodyStub, CachedActorPointer, ptr<HierarchyComponent>>(
			ComponentGuid { componentGuid },
			CachedActorPointer { owner->guid(), owner },
			nullptr
		);
		ASSERT_TRUE(registry()->contains(componentGuid));

		const ActorGuid guids[] { generate_actor_guid() };
		writeActors(_archive, guids, -1, componentGuid);
		const auto level = hydrateLevel();

		EXPECT_TRUE(level->getActors().empty());
		EXPECT_TRUE(hasCode(_sink, "duplicate-identity"));
		EXPECT_FALSE(registry()->contains(guids[0]));
		EXPECT_TRUE(registry()->contains(componentGuid));
	}

	TEST_F(LoadBoundary, RegistryContainsArchetypeActor) {
		const auto actor = CreateActor(&HeliogrimApi::TestTank::actorClass());
		ASSERT_NE(actor, nullptr);
		EXPECT_TRUE(registry()->contains(actor->guid()));
	}

	TEST_F(LoadBoundary, DuplicateGuidWithoutContextRejected) {
		const auto guid = generate_actor_guid();
		const ActorGuid guids[] { guid, guid };
		writeActors(_archive, guids);
		_archive.seek(0);

		const auto root = RootScopedSlot { _archive, ScopedSlotStateFlag::eClean, nullptr };
		auto level = make_root_like_level();
		access::Structure<Level>::hydrate(root.asStruct(), *level);

		EXPECT_TRUE(level->getActors().empty());
		EXPECT_FALSE(registry()->contains(guid));
	}

	TEST_F(LoadBoundary, UnknownRootTypeRejected) {
		{
			auto structured = StructuredArchive { _archive };
			auto root = structured.insertRootSlot().intoStruct();
			root.insertSlot<u64>(Symbols::Type) << 0xDEAD'0000'0000'0001uLL;
		}
		const auto level = loadLevel(_archive, _sink);

		EXPECT_FALSE(level.has_value());
		EXPECT_TRUE(hasCode(_sink, "unknown-type"));
		EXPECT_TRUE(_sink.hasErrors());
	}

	TEST_F(LoadBoundary, ScalarRootMalformed) {
		{
			auto structured = StructuredArchive { _archive };
			structured.insertRootSlot().intoIntegral<u64>() << 7uLL;
		}
		const auto level = loadLevel(_archive, _sink);

		EXPECT_FALSE(level.has_value());
		EXPECT_TRUE(hasCode(_sink, "malformed-slot"));
		EXPECT_TRUE(_sink.hasErrors());
	}

	TEST_F(LoadBoundary, EmptyArchiveTruncated) {
		const auto level = loadLevel(_archive, "Sandbox", _sink);

		EXPECT_FALSE(level.has_value());
		EXPECT_TRUE(hasCode(_sink, "truncated"));
		EXPECT_TRUE(_sink.hasErrors());
	}

	TEST_F(LoadBoundary, AoSLevelRootRejected) {
		{
			auto structured = StructuredArchive { _archive };
			structured.insertRootSlot().intoSeq().addRecordSlot().intoIntegral<u64>() << 1uLL;
		}
		const auto level = loadLevel(_archive, "Sandbox", _sink);

		ASSERT_FALSE(level.has_value());
		EXPECT_EQ(level.error().front().code()->key, "malformed-slot");
		EXPECT_NE(engine::diag::render(level.error().front()).find("level root must be a record"), String::npos);
	}

	TEST_F(LoadBoundary, MalformedLevelActorSeqRejected) {
		{
			auto structured = StructuredArchive { _archive };
			auto root = structured.insertRootSlot().intoStruct();
			access::Structure<math::Bounding>::serialize(math::Bounding {}, root.insertStructSlot("bounding"));
			auto actors = root.insertRecordSlot(Symbols::Components).intoSeq();
			actors.addRecordSlot().intoIntegral<u64>() << 1uLL;
		}
		// The seq is the last slot: [count u64][u64 record (tag + 8)]; a count of all ones exceeds its payload.
		auto count = ~0uLL;
		_archive.seek(_archive.totalSize() - 17LL);
		_archive.serializeBytes(&count, sizeof(u64), ArchiveStreamMode::eStore);
		const auto level = loadLevel(_archive, "Sandbox", _sink);

		EXPECT_FALSE(level.has_value());
		EXPECT_TRUE(hasCode(_sink, "malformed-slot"));
	}

	TEST_F(LoadBoundary, UnknownLevelKeyMarksLevel) {
		{
			auto structured = StructuredArchive { _archive };
			auto root = structured.insertRootSlot().intoStruct();
			access::Structure<math::Bounding>::serialize(math::Bounding {}, root.insertStructSlot("bounding"));
			std::ignore = root.insertRecordSlot(Symbols::Components).intoSeq();
			root.insertSlot<u64>("futureLevelField") << 3uLL;
		}
		const auto level = loadLevel(_archive, "Sandbox", _sink);

		ASSERT_TRUE(level.has_value());
		EXPECT_TRUE(level.value()->carriesUnknownData());
		EXPECT_TRUE(hasCode(_sink, "unknown-field"));
		EXPECT_FALSE(_sink.hasErrors());
		EXPECT_FALSE(checkSaveAllowed(*level.value()).has_value());
	}

	TEST_F(LoadBoundary, AoSAssetRoot) {
		{
			auto structured = StructuredArchive { _archive };
			auto seq = structured.insertRootSlot().intoSeq();
			seq.addRecordSlot().intoIntegral<u64>() << 1uLL;
		}
		_archive.seek(0);
		const auto root = RootScopedSlot { _archive, ScopedSlotStateFlag::eClean, &_ctx };
		EXPECT_EQ(classifyRoot(root, _ctx), RootKind::eAoS);
		EXPECT_FALSE(_ctx.failed());
	}

	TEST_F(LoadBoundary, SoARootRoundTrip) {
		{
			auto structured = StructuredArchive { _archive };
			auto root = structured.insertRootSlot().intoStruct();
			insertSoaRoot(root, typeOf(testActorType), 3uLL);
			root.insertSlot<u32, Vector>("a") << Vector<u32> { 1u, 2u, 3u };
			root.insertSlot<u32, Vector>("zz") << Vector<u32> { 9u, 9u, 9u };
		}
		_archive.seek(0);
		{
			const auto root = RootScopedSlot { _archive, ScopedSlotStateFlag::eClean, &_ctx };
			ASSERT_EQ(classifyRoot(root, _ctx), RootKind::eSoA);
			const auto record = root.asStruct();
			u64 type = 0uLL;
			u64 count = 0uLL;
			record.getSlot<u64>(Symbols::Type) >> type;
			record.getSlot<u64>(soaCountKey) >> count;
			EXPECT_EQ(type, testActorType);
			ASSERT_EQ(count, 3uLL);
			EXPECT_EQ(readSoaColumn<u32>(record, "a", count), (Vector<u32> { 1u, 2u, 3u }));
			EXPECT_EQ(readSoaColumn<u32>(record, "b", count), (Vector<u32> { 0u, 0u, 0u }));
			(void)record.reportUnreadKeys();
		}
		EXPECT_TRUE(_ctx.finish(_sink, "soa").has_value());
		EXPECT_TRUE(hasCode(_sink, "unknown-field"));
		EXPECT_FALSE(_sink.hasErrors());
	}

	TEST_F(LoadBoundary, SoAColumnLengthMismatch) {
		{
			auto structured = StructuredArchive { _archive };
			auto root = structured.insertRootSlot().intoStruct();
			insertSoaRoot(root, typeOf(testActorType), 3uLL);
			root.insertSlot<u32, Vector>("a") << Vector<u32> { 1u, 2u };
		}
		_archive.seek(0);
		{
			const auto root = RootScopedSlot { _archive, ScopedSlotStateFlag::eClean, &_ctx };
			ASSERT_EQ(classifyRoot(root, _ctx), RootKind::eSoA);
			EXPECT_EQ(readSoaColumn<u32>(root.asStruct(), "a", 3uLL).size(), 3uLL);
		}
		auto result = _ctx.finish(_sink, "soa");
		ASSERT_FALSE(result.has_value());
		for (auto& diagnostic : result.error()) {
			_sink.emit(::hg::move(diagnostic));
		}
		EXPECT_TRUE(hasCode(_sink, "malformed-slot"));
	}

	TEST_F(LoadBoundary, SoACountOverBudgetRefused) {
		{
			auto structured = StructuredArchive { _archive };
			auto root = structured.insertRootSlot().intoStruct();
			insertSoaRoot(root, typeOf(testActorType), 1uLL << 60);
		}
		_archive.seek(0);
		{
			const auto root = RootScopedSlot { _archive, ScopedSlotStateFlag::eClean, &_ctx };
			ASSERT_EQ(classifyRoot(root, _ctx), RootKind::eSoA);
			EXPECT_TRUE(readSoaColumn<u64>(root.asStruct(), "a", 1uLL << 60).empty());
		}
		EXPECT_TRUE(_ctx.failed());
		auto result = _ctx.finish(_sink, "soa");
		ASSERT_FALSE(result.has_value());
		for (auto& diagnostic : result.error()) {
			_sink.emit(::hg::move(diagnostic));
		}
		EXPECT_TRUE(hasCode(_sink, "limit-exceeded"));
	}

	TEST_F(LoadBoundary, SoANewerRevisionRefused) {
		{
			auto structured = StructuredArchive { _archive };
			auto root = structured.insertRootSlot().intoStruct();
			insertSoaRoot(root, typeOf(testActorType), 1uLL);
			root.insertSlot<u16>("@rev") << u16 { 1 };
		}
		_archive.seek(0);
		{
			const auto root = RootScopedSlot { _archive, ScopedSlotStateFlag::eClean, &_ctx };
			EXPECT_FALSE(classifyRoot(root, _ctx).has_value());
		}
		auto result = _ctx.finish(_sink, "soa");
		ASSERT_FALSE(result.has_value());
		for (auto& diagnostic : result.error()) {
			_sink.emit(::hg::move(diagnostic));
		}
		EXPECT_TRUE(hasCode(_sink, "revision-newer"));
	}

	TEST_F(LoadBoundary, ShortActorGuidRejected) {
		{
			auto structured = StructuredArchive { _archive };
			auto root = structured.insertRootSlot().intoStruct();
			access::Structure<math::Bounding>::serialize(math::Bounding {}, root.insertStructSlot("bounding"));
			auto actors = root.insertRecordSlot(Symbols::Components).intoSeq();
			auto actor = actors.addRecordSlot().intoStruct();
			actor.insertSlot<u64>(Symbols::Type) << testActorType;
			actor.insertSlot<String>(Symbols::Guid) << String { "abcd" };
			(void)actor.insertRecordSlot(Symbols::Components).intoSeq();
		}
		const auto level = hydrateLevel();

		EXPECT_TRUE(level->getActors().empty());
		EXPECT_TRUE(hasCode(_sink, "malformed-slot"));
		EXPECT_TRUE(_sink.hasErrors());
	}

	TEST_F(LoadBoundary, ArchetypeComponentNewerRevisionMarksActor) {
		{
			auto structured = StructuredArchive { _archive };
			auto root = structured.insertRootSlot().intoStruct();
			access::Structure<math::Bounding>::serialize(math::Bounding {}, root.insertStructSlot("bounding"));
			auto actors = root.insertRecordSlot(Symbols::Components).intoSeq();
			auto actor = actors.addRecordSlot().intoStruct();
			actor.insertSlot<u64>(Symbols::Type) << HeliogrimApi::TestTank::typeId.data;
			insert_guid_slot(actor, generate_actor_guid());
			actor.insertSlot<String>("label") << String { "tank" };
			auto components = actor.insertStructSlot(Symbols::Components);
			auto turret = components.insertStructSlot("body/turret");
			turret.insertSlot<u64>(Symbols::Type) << HeliogrimApi::TurretStub::typeId.data;
			turret.insertSlot<u16>("@rev") << u16 { 1 };
		}
		const auto level = hydrateLevel();

		ASSERT_EQ(level->getActors().size(), 1uLL);
		EXPECT_TRUE(level->getActors().front()->carriesUnknownData());
		EXPECT_TRUE(hasCode(_sink, "revision-newer"));
	}
}
