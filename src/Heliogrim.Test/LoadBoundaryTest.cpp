#include "pch.h"

/**/

#include <Engine.ACS/Registry.hpp>
#include <Engine.ACS/Traits.hpp>
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

namespace LoadBoundaryModule {
	constexpr u64 testActorType = 0x7E57'0000'0000'0011uLL;
	constexpr u64 testComponentType = 0x7E57'0000'0000'0012uLL;

	struct TestAccess {
		template <typename = void>
		static ptr<Actor> instantiate(nmpt<engine::acs::Registry> allocator_, cref<ActorGuid> guid_) {
			return ActorScheme<Actor>::instantiate<void>(allocator_, guid_);
		}

		static void hydrate(cref<StructScopedSlot>, ref<Actor>) {}
	};

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
		}

		void TearDown() override {
			_engine.reset();
		}

		[[nodiscard]] nmpt<engine::acs::Registry> registry() const {
			return _engine->getActors()->getRegistry();
		}

		void writeLevel(const u16 actorRev_, std::span<const ComponentSpec> components_) {
			auto structured = StructuredArchive { _archive };
			auto root = structured.insertRootSlot().intoStruct();
			access::Structure<math::Bounding>::serialize(math::Bounding {}, root.insertStructSlot("bounding"));
			auto actors = root.insertRecordSlot(Symbols::Components).intoSeq();
			auto actor = actors.addRecordSlot().intoStruct();
			actor.insertSlot<u64>(Symbols::Type) << testActorType;
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
			std::ignore = _ctx.finish(_sink, "level");
			return level;
		}
	};

	TEST_F(LoadBoundary, RegistryContainsCreatedActorOnly) {
		const auto guid = generate_actor_guid();
		const auto other = generate_actor_guid();
		const auto* const actor = ActorScheme<Actor>::instantiate<void>(registry(), guid);

		ASSERT_NE(actor, nullptr);
		EXPECT_TRUE(registry()->contains(guid));
		EXPECT_FALSE(registry()->contains(other));
	}
}
