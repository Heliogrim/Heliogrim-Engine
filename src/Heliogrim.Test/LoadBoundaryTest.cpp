#include "pch.h"

/**/


#include "SpawnEngine.hpp"
#include "TestArchetypes.hpp"

import Engine.ACS;
import Engine.Assets.Type;
import Engine.Diagnostics;
import Engine.Level;
import Engine.Resource.Archive;
import Engine.Serialization;
import Engine.Serialization.Structures;

using namespace hg::engine::serialization;
using namespace hg::engine::resource;
using namespace hg::engine::core;
using namespace hg;
using hg::engine::assets::FontAsset;

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
				::hg::engine::acs::MakeArchetypeFnSet(&HeliogrimApi::TestTank::actorClass())
			);
			hydratedValue = 0u;
		}

		void TearDown() override {
			_engine.reset();
		}

		[[nodiscard]] nmpt<engine::acs::Registry> registry() const {
			return engine::ActorModule::get()->getRegistry();
		}

		void writeLevel(const u16 actorRev_, std::span<const ComponentSpec> components_, const u64 actorType_ = testActorType) {
			auto structured = StructuredArchive { _archive };
			auto root = structured.insertRootSlot().intoStruct();
			access::Structure<math::Bounding>::serialize(math::Bounding {}, root.insertStructSlot("bounding"));
			auto actors = root.insertRecordSlot(Symbols::Components).intoSeq();
			auto actor = actors.addRecordSlot().intoStruct();
			actor.insertSlot<u64>(Symbols::Type) << actorType_;
			if (actorRev_ > 0) {
				actor.insertSlot<u16>(Symbols::Revision) << actorRev_;
			}
			actor.insertSlot<string>(Symbols::Guid) << string(16uLL, static_cast<char>(++guidSeed));
			auto seq = actor.insertRecordSlot(Symbols::Components).intoSeq();
			for (const auto& spec : components_) {
				auto component = seq.addRecordSlot().intoStruct();
				if (spec.withType) {
					component.insertSlot<u64>(Symbols::Type) << spec.type;
				}
				if (spec.rev > 0) {
					component.insertSlot<u16>(Symbols::Revision) << spec.rev;
				}
				component.insertSlot<s8>(Symbols::Parent) << spec.parent;
			}
		}

		[[nodiscard]] Arci<engine::core::Level> hydrateLevel() {
			_archive.seek(0);
			const auto root = RootScopedSlot { _archive, ScopedSlotStateFlag::eClean, &_ctx };
			auto level = make_root_like_level();
			access::Structure<engine::core::Level>::hydrate(root.asStruct(), *level);
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
		access::Structure<engine::core::Level>::hydrate(root.asStruct(), *level);

		EXPECT_TRUE(level->getActors().empty());
		EXPECT_FALSE(registry()->contains(guid));
	}

	TEST_F(LoadBoundary, UnknownRootTypeRejected) {
		{
			auto structured = StructuredArchive { _archive };
			auto root = structured.insertRootSlot().intoStruct();
			root.insertSlot<u64>(Symbols::Type) << 0xDEAD'0000'0000'0001uLL;
		}
		const auto level = loadLevel(_archive, "Sandbox", _sink);

		EXPECT_FALSE(level.has_value());
		EXPECT_TRUE(hasCode(_sink, "unknown-type"));
		EXPECT_TRUE(_sink.hasErrors());
	}

	TEST_F(LoadBoundary, ScalarRootMalformed) {
		{
			auto structured = StructuredArchive { _archive };
			structured.insertRootSlot().intoIntegral<u64>() << 7uLL;
		}
		const auto level = loadLevel(_archive, "Sandbox", _sink);

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
			root.insertSlot<u16>(Symbols::Revision) << u16 { 1 };
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
			turret.insertSlot<u16>(Symbols::Revision) << u16 { 1 };
		}
		const auto level = hydrateLevel();

		ASSERT_EQ(level->getActors().size(), 1uLL);
		EXPECT_TRUE(level->getActors().front()->carriesUnknownData());
		EXPECT_TRUE(hasCode(_sink, "revision-newer"));
	}

	TEST_F(LoadBoundary, ContextNoteNamesLevel) {
		{
			auto structured = StructuredArchive { _archive };
			structured.insertRootSlot().intoIntegral<u64>() << 7uLL;
		}
		const auto level = loadLevel(_archive, "Sandbox", _sink);

		ASSERT_FALSE(level.has_value());
		ASSERT_FALSE(level.error().empty());
		const auto& notes = level.error().front().notes();
		EXPECT_NE(std::ranges::find(notes, String { "while loading level 'Sandbox'" }), notes.end());
	}

	TEST_F(LoadBoundary, StructurallyInvalidLevelRejectedBeforeHydrate) {
		{
			auto structured = StructuredArchive { _archive };
			auto root = structured.insertRootSlot().intoStruct();
			root.insertSlot<String>("name") << String { "first" };
			root.insertSlot<String>("name") << String { "second" };
		}
		const auto level = loadLevel(_archive, "Sandbox", _sink);

		ASSERT_FALSE(level.has_value());
		ASSERT_FALSE(level.error().empty());
		EXPECT_NE(level.error().front().message().find("duplicate struct key"), String::npos) << level.error().front().message();
	}

	TEST_F(LoadBoundary, ContextFillsAssetUrl) {
		{
			auto structured = StructuredArchive { _archive };
			auto root = structured.insertRootSlot().intoStruct();
			root.insertSlot<u64>(Symbols::Type) << 0xDEAD'0000'0000'0001uLL;
		}
		const auto level = loadLevel(_archive, "Sandbox", _sink);

		ASSERT_FALSE(level.has_value());
		const auto* at = std::get_if<engine::diag::AssetPath>(&level.error().front().primary());
		ASSERT_NE(at, nullptr);
		EXPECT_EQ(at->url, "Sandbox");
	}

	/* Writes a font asset record by hand so tests control `@rev` and stray keys. */
	void writeFont(ref<BufferArchive> archive_, const u16 rev_, const StringView sourcesKey_, const bool strayKey_) {
		auto structured = StructuredArchive { archive_ };
		auto root = structured.insertRootSlot().intoStruct();
		if (rev_ > 0) {
			root.insertSlot<u16>(Symbols::Revision) << rev_;
		}
		insert_guid_slot(root, AssetGuid {});
		insert_typeId_slot(root, FontAsset::typeId);
		root.insertSlot<String>("name") << String { "font" };
		root.insertSlot<String, Vector>(sourcesKey_) << Vector<String> {};
		if (strayKey_) {
			root.insertSlot<u32>("futureField") << 1u;
		}
	}

	struct FontLoad {
		FontAsset font {};
		bool hydrated = false;
		Result<void, engine::diag::Diagnostics> finished {};
	};

	void loadFont(ref<BufferArchive> archive_, cref<MigrationRegistry> migrations_, ref<engine::diag::CollectingSink> sink_, ref<FontLoad> out_) {
		auto ctx = LoadContext { static_cast<u64>(archive_.totalSize()), "fonts/font" };
		archive_.seek(0);
		{
			const auto root = RootScopedSlot { archive_, ScopedSlotStateFlag::eClean, &ctx };
			out_.hydrated = engine::assets::hydrateArchivedAsset(migrations_, root.asStruct(), ctx, out_.font);
		}
		out_.finished = ctx.finish(sink_, "while loading asset 'fonts/font'");
	}

	TEST_F(LoadBoundary, LegacyAssetKeysStillLoad) {
		const auto guid = Guid { 1u, 2u, 3u, 4uLL };
		{
			auto structured = StructuredArchive { _archive };
			auto root = structured.insertRootSlot().intoStruct();
			access::Structure<Guid>::serialize(guid, root.insertSlot<void>("__guid__").intoStruct());
			root.insertSlot<u64>("__type__") << engine::assets::LevelAsset::typeId.data;
			root.insertSlot<String>("name") << String { "legacy" };
			root.insertSlot<String>("levelData") << String {};
		}
		auto ctx = LoadContext { static_cast<u64>(_archive.totalSize()), "levels/legacy" };
		_archive.seek(0);
		engine::assets::LevelAsset asset {};
		{
			const auto root = RootScopedSlot { _archive, ScopedSlotStateFlag::eClean, &ctx };
			EXPECT_TRUE(engine::assets::hydrateArchivedAsset(_engine->_serialization->migrations(), root.asStruct(), ctx, asset));
		}
		EXPECT_TRUE(ctx.finish(_sink, "while loading asset 'levels/legacy'").has_value());
		EXPECT_EQ(static_cast<Guid>(asset.getAssetGuid()), guid);
		EXPECT_EQ(asset.getAssetName(), "legacy");
	}

	TEST_F(LoadBoundary, AssetUnreadKeyMarksAndRefusesStore) {
		writeFont(_archive, 0, "sources", true);
		FontLoad load {};
		loadFont(_archive, _engine->_serialization->migrations(), _sink, load);

		EXPECT_TRUE(load.hydrated);
		EXPECT_TRUE(load.finished.has_value());
		EXPECT_TRUE(hasCode(_sink, "unknown-field"));
		EXPECT_TRUE(load.font.carriesUnknownData());

		const nmpt<const engine::assets::Asset> assets[] { &load.font };
		const auto store = engine::assets::checkStoreAllowed(assets);
		ASSERT_FALSE(store.has_value());
		EXPECT_EQ(store.error().front().code()->key, "save-refused");
	}

	TEST_F(LoadBoundary, AssetNewerRevisionRefused) {
		writeFont(_archive, 1, "sources", false);
		FontLoad load {};
		loadFont(_archive, _engine->_serialization->migrations(), _sink, load);

		EXPECT_FALSE(load.hydrated);
		ASSERT_FALSE(load.finished.has_value());
		EXPECT_EQ(load.finished.error().front().code()->key, "revision-newer");
	}

	Result<void, MigrateError> renameSources(ref<const StructScopedSlot> old_, ref<StructScopedSlot> new_) {
		forwardEntries(old_, new_, { "fontSources" });
		Vector<String> sources {};
		old_.getSlot<String, Vector>("fontSources") >> sources;
		new_.insertSlot<String, Vector>("sources") << sources;
		return {};
	}

	TEST_F(LoadBoundary, AssetOlderRevisionMigrates) {
		const Migration steps[] { { FontAsset::typeId, 0, &renameSources } };
		_engine->_serialization->migrations().add(steps);
		writeFont(_archive, 0, "fontSources", false);
		FontLoad load {};
		loadFont(_archive, _engine->_serialization->migrations(), _sink, load);

		EXPECT_TRUE(load.hydrated);
		EXPECT_TRUE(load.finished.has_value());
		EXPECT_FALSE(load.font.carriesUnknownData());
		EXPECT_FALSE(hasCode(_sink, "unknown-field"));
	}

	TEST_F(LoadBoundary, LevelAssetBadLevelDataRejected) {
		const auto load = [this](const StringView value_, ref<engine::assets::LevelAsset> asset_) {
			_archive = BufferArchive {};
			{
				auto structured = StructuredArchive { _archive };
				auto root = structured.insertRootSlot().intoStruct();
				insert_guid_slot(root, AssetGuid {});
				insert_typeId_slot(root, engine::assets::LevelAsset::typeId);
				root.insertSlot<String>("name") << String { "level" };
				root.insertSlot<String>("levelData") << String { value_ };
			}
			auto ctx = LoadContext { static_cast<u64>(_archive.totalSize()), "levels/level" };
			_archive.seek(0);
			bool hydrated = false;
			{
				const auto root = RootScopedSlot { _archive, ScopedSlotStateFlag::eClean, &ctx };
				hydrated = engine::assets::hydrateArchivedAsset(_engine->_serialization->migrations(), root.asStruct(), ctx, asset_);
			}
			return std::make_pair(hydrated, ctx.finish(_sink, "while loading asset 'levels/level'"));
		};
		const auto unset = engine::assets::LevelAsset {}.getLevelData().encode();

		for (const auto bad : {
			     "file://nope", "archive://zz", "archive://99999999999999999999", "archive://1-2-3",
			     "archive://x-2-3-4", "archive://1a-2-3-4", "archive://-2-3-4"
		     }) {
			engine::assets::LevelAsset asset {};
			const auto [hydrated, finished] = load(bad, asset);
			EXPECT_FALSE(hydrated) << bad;
			EXPECT_EQ(asset.getLevelData().encode(), unset) << bad;
			ASSERT_FALSE(finished.has_value()) << bad;
			EXPECT_EQ(finished.error().front().code()->key, "invalid-value") << bad;
		}

		engine::assets::LevelAsset asset {};
		const auto [hydrated, finished] = load("archive://1-2-3-4", asset);
		EXPECT_TRUE(hydrated);
		EXPECT_TRUE(finished.has_value());
		EXPECT_NE(asset.getLevelData().encode(), unset);
	}
}

namespace LoadBoundaryModule {
	TEST_F(LoadBoundary, SecondRootComponentRejected) {
		constexpr u64 rootType = 0x7E57'0000'0000'00A1uLL;
		_engine->_serialization->addComponentScheme(
			typeOf(rootType),
			ComponentFnSet {
				.serialize = [](cref<LogicComponent>, mref<StructScopedSlot>) {},
				.deserialize = [](cref<RecordScopedSlot>, nmpt<engine::acs::Registry>, ref<Actor> owner_, nmpt<HierarchyComponent> parent_) {
					return VolatileComponent<> {
						new HierarchyComponent { generate_component_guid(), CachedActorPointer { owner_.guid(), &owner_ }, parent_.get() },
						[](ptr<LogicComponent> obj_) { delete obj_; }
					};
				}
			}
		);

		// The kept root is pooled so actor teardown frees it; the rejected one is plain heap and dropped by its own deleter.
		constexpr u64 bodyType = 0x7E57'0000'0000'00A2uLL;
		_engine->_serialization->addComponentScheme(
			typeOf(bodyType),
			ComponentFnSet {
				.serialize = [](cref<LogicComponent>, mref<StructScopedSlot>) {},
				.deserialize = [](cref<RecordScopedSlot>, nmpt<engine::acs::Registry> allocator_, ref<Actor> owner_, nmpt<HierarchyComponent> parent_) {
					return VolatileComponent<> { ComponentScheme<HeliogrimApi::BodyStub>::instantiate<void>(allocator_, ComponentGuid { generate_component_guid() }, owner_, parent_).release() };
				}
			}
		);

		const ComponentSpec components[] { { .type = bodyType }, { .type = rootType } };
		writeLevel(0, components);
		std::ignore = hydrateLevel();

		EXPECT_TRUE(hasCode(_sink, "invalid-value"));
		EXPECT_TRUE(_sink.hasErrors());
	}
}
