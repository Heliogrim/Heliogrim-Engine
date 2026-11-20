#include "pch.h"

/**/

#include <Engine.Diagnostics/Sink.hpp>
#include <Engine.Level/Level.hpp>
#include <Engine.Resource.Archive/BufferArchive.hpp>
#include <Engine.Serialization/SerializationModule.hpp>
#include <Engine.Serialization/Access/Structure.hpp>
#include <Engine.Serialization/Archive/StructuredArchive.hpp>
#include <Engine.Serialization/Load/LoadContext.hpp>
#include <Engine.Serialization/Scheme/ActorScheme.hpp>
#include <Engine.Serialization/Structure/RootScopedSlot.hpp>
#include <Engine.Serialization/Structure/SeqScopedSlot.hpp>
#include <Heliogrim/Actor/Actor.hpp>

#include "SpawnEngine.hpp"

using namespace hg::engine::serialization;
using namespace hg::engine::resource;
using namespace hg::engine::core;
using namespace hg;

namespace UnknownDataModule {
	constexpr u64 testActorType = 0x7E57'0000'0000'0001uLL;
	constexpr u64 testComponentType = 0x7E57'0000'0000'0002uLL;
	constexpr u64 missingType = 0x7E57'0000'0000'00FFuLL;

	/* Set by a test to read keys of the actor record through a second, independent alias. */
	ptr<const RecordScopedSlot> aliasSource = nullptr;
	Vector<String> aliasReads {};

	/* Builds a plain actor and reads nothing itself, so only the scheme's reads (and the alias reads) count. */
	struct TestAccess {
		template <typename = void>
		static ptr<Actor> instantiate(nmpt<engine::acs::Registry> allocator_, cref<ActorGuid> guid_) {
			return ActorScheme<Actor>::instantiate<void>(allocator_, guid_);
		}

		static void hydrate(cref<StructScopedSlot>, ref<Actor>) {
			if (aliasSource == nullptr) {
				return;
			}
			const auto alias = aliasSource->asStruct();
			for (const auto& key : aliasReads) {
				u64 ignored {};
				alias.getSlot<u64>(key) >> ignored;
			}
		}
	};

	inline u8 guidSeed = 0;

	void writeActor(ref<StructScopedSlot> actor_, const u64 actorType_, const u64 componentType_, const bool componentExtra_) {
		actor_.insertSlot<u64>(Symbols::Type) << actorType_;
		actor_.insertSlot<string>(Symbols::Guid) << string(16uLL, static_cast<char>(++guidSeed)); // fresh guid: the actor pool outlives a test and keeps guids
		auto components = actor_.insertRecordSlot(Symbols::Components).intoSeq();
		if (componentType_ == 0uLL) {
			return;
		}
		auto component = components.addRecordSlot().intoStruct();
		component.insertSlot<u64>(Symbols::Type) << componentType_;
		component.insertSlot<s8>(Symbols::Parent) << s8 { -1 };
		if (componentExtra_) {
			component.insertSlot<u64>("futureField") << 7uLL;
		}
	}

	[[nodiscard]] bool hasCode(cref<engine::diag::CollectingSink> sink_, StringView key_) {
		for (const auto& item : sink_.items()) {
			if (item.code() != nullptr && item.code()->key == key_) {
				return true;
			}
		}
		return false;
	}

	class UnknownData : public ::testing::Test {
	protected:
		uptr<HeliogrimApi::test_engine::SpawnEngine> _engine;
		BufferArchive _archive {};
		LoadContext _ctx { 1uLL << 20 };
		engine::diag::CollectingSink _sink {};

		void SetUp() override {
			_engine = make_uptr<HeliogrimApi::test_engine::SpawnEngine>();
			aliasSource = nullptr;
			aliasReads.clear();

			type_id actorType {};
			actorType.data = testActorType;
			_engine->_serialization->addActorScheme(
				actorType,
				ActorFnSet {
					.serialize = [](cref<Actor>, mref<StructScopedSlot>) {},
					.deserialize = [](cref<RecordScopedSlot> slot_, nmpt<engine::acs::Registry> allocator_) -> ptr<Actor> {
						return ActorScheme<Actor>::deserialize<TestAccess>(slot_, allocator_);
					}
				}
			);

			type_id componentType {};
			componentType.data = testComponentType;
			_engine->_serialization->addComponentScheme(
				componentType,
				ComponentFnSet {
					.serialize = [](cref<LogicComponent>, mref<StructScopedSlot>) {},
					// Reads no field of its own; returns no component, which the actor scheme drops.
					.deserialize = [](cref<RecordScopedSlot>, nmpt<engine::acs::Registry>, ref<Actor>, nmpt<HierarchyComponent>) {
						return VolatileComponent<> { static_cast<ptr<LogicComponent>>(nullptr) };
					}
				}
			);
		}

		void TearDown() override {
			aliasSource = nullptr;
			_engine.reset();
		}

		[[nodiscard]] nmpt<engine::acs::Registry> registry() const {
			return _engine->getActors()->getRegistry();
		}

		/* Level root with one actor record. */
		void writeLevel(const u64 actorType_, const u64 componentType_, const bool componentExtra_) {
			auto structured = StructuredArchive { _archive };
			auto root = structured.insertRootSlot().intoStruct();
			access::Structure<math::Bounding>::serialize(math::Bounding {}, root.insertStructSlot("bounding"));
			auto actors = root.insertRecordSlot(Symbols::Components).intoSeq();
			auto actor = actors.addRecordSlot().intoStruct();
			writeActor(actor, actorType_, componentType_, componentExtra_);
		}

		[[nodiscard]] Arci<Level> hydrateLevel() {
			_archive.seek(0);
			const auto root = RootScopedSlot { _archive, ScopedSlotStateFlag::eClean, &_ctx };
			auto level = make_root_like_level();
			access::Structure<Level>::hydrate(root.asStruct(), *level);
			EXPECT_TRUE(_ctx.finish(_sink, "level").has_value());
			return level;
		}
	};

	TEST_F(UnknownData, ExtraFieldWarnsAndMarks) {
		{
			auto structured = StructuredArchive { _archive };
			auto root = structured.insertRootSlot().intoStruct();
			writeActor(root, testActorType, 0uLL, false);
			root.insertSlot<u64>("futureField") << 3uLL;
		}

		_archive.seek(0);
		const auto root = RootScopedSlot { _archive, ScopedSlotStateFlag::eClean, &_ctx };
		const auto actor = ActorScheme<Actor>::deserialize<TestAccess>(root, registry());

		ASSERT_NE(actor, nullptr);
		EXPECT_TRUE(actor->carriesUnknownData());
		EXPECT_TRUE(_ctx.finish(_sink, "actor").has_value());
		EXPECT_TRUE(hasCode(_sink, "unknown-field"));
		EXPECT_FALSE(_sink.hasErrors());
	}

	TEST_F(UnknownData, AliasedReadsNotUnknown) {
		{
			auto structured = StructuredArchive { _archive };
			auto root = structured.insertRootSlot().intoStruct();
			writeActor(root, testActorType, 0uLL, false);
			root.insertSlot<u64>("alpha") << 1uLL;
			root.insertSlot<u64>("beta") << 2uLL;
		}

		_archive.seek(0);
		const auto root = RootScopedSlot { _archive, ScopedSlotStateFlag::eClean, &_ctx };

		// The scheme reads guid and components through its alias, hydrate reads the rest through two more.
		aliasSource = &root;
		aliasReads = { "alpha" };
		{
			const auto other = root.asStruct();
			u64 ignored {};
			other.getSlot<u64>("beta") >> ignored;
		}
		const auto actor = ActorScheme<Actor>::deserialize<TestAccess>(root, registry());

		ASSERT_NE(actor, nullptr);
		EXPECT_FALSE(actor->carriesUnknownData());
		EXPECT_TRUE(_ctx.finish(_sink, "actor").has_value());
		EXPECT_FALSE(hasCode(_sink, "unknown-field"));
	}

	TEST_F(UnknownData, NestedComponentFieldRefusesLevelSave) {
		writeLevel(testActorType, testComponentType, true);
		const auto level = hydrateLevel();

		ASSERT_EQ(level->getActors().size(), 1uLL);
		EXPECT_TRUE(level->getActors().front()->carriesUnknownData());
		EXPECT_TRUE(hasCode(_sink, "unknown-field"));

		const auto saved = checkSaveAllowed(*level);
		ASSERT_FALSE(saved.has_value());
		ASSERT_EQ(saved.error().size(), 1uLL);
		EXPECT_EQ(String { saved.error().front().code()->key }, "save-refused");
	}

	TEST_F(UnknownData, UnknownActorTypeSkippedAndRefused) {
		writeLevel(missingType, 0uLL, false);
		const auto level = hydrateLevel();

		EXPECT_TRUE(level->getActors().empty());
		EXPECT_TRUE(level->carriesUnknownData());
		EXPECT_TRUE(hasCode(_sink, "unknown-type"));

		const auto saved = checkSaveAllowed(*level);
		ASSERT_FALSE(saved.has_value());
		EXPECT_EQ(String { saved.error().front().code()->key }, "save-refused");
	}

	TEST_F(UnknownData, UnknownComponentSkipsWholeActor) {
		writeLevel(testActorType, missingType, false);
		const auto level = hydrateLevel();

		EXPECT_TRUE(level->getActors().empty());
		EXPECT_TRUE(level->carriesUnknownData());
		EXPECT_TRUE(hasCode(_sink, "unknown-type"));
		EXPECT_FALSE(checkSaveAllowed(*level).has_value());
	}
}
