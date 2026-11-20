#include "pch.h"

/**/

#include <Engine.ACS/Registry.hpp>
#include <Engine.ACS/Archetype/LayoutInfo.hpp>
#include <Engine.Assets.Type/Geometry/StaticGeometry.hpp>
#include <Engine.Serialization/Access/Structure.hpp>
#include <Engine.Serialization/Archive/StructuredArchive.hpp>
#include <Engine.Serialization/Load/LoadContext.hpp>
#include <Engine.Diagnostics/Sink.hpp>
#include <Engine.Serialization/Structure/FloatScopedSlot.hpp>
#include <Engine.Serialization/Structure/IntegralScopedSlot.hpp>
#include <Engine.Serialization/Structure/RecordScopedSlot.hpp>
#include <Engine.Serialization/Structure/StructScopedSlot.hpp>
#include <Engine.Serialization.Structures/Symbols.hpp>
#include <Engine.Serialization.Structures/Common/Guid.hpp>
#include <Engine.Serialization.Structures/Common/TypeId.hpp>
#include <Heliogrim/Actor/Actor.hpp>
#include <Engine.Serialization/Scheme/ActorScheme.hpp>
#include <Engine.Serialization/Scheme/ComponentScheme.hpp>
#include <Engine.Serialization/Structure/SeqScopedSlot.hpp>
#include <Heliogrim/Actor/ActorRef.hpp>
#include <Heliogrim/Actor/Archetype/ActorClass.hpp>
#include <Heliogrim/Actor/Archetype/ArchetypeDefinition.inl.hpp>
#include <Heliogrim/Actor/Archetype/ArchetypeFnSet.hpp>
#include <Heliogrim/Actor/Geometry/StaticGeometryActor.hpp>
#include <Heliogrim/Component/ComponentRef.hpp>
#include <Heliogrim/Component/Scene/Geometry/StaticGeometryComponent.Layout.hpp>
#include <Heliogrim/Universe/Universe.hpp>

#include "SpawnEngine.hpp"
#include "TestArchetypes.hpp"

import Engine.Common;
import Engine.Resource.Archive;

using namespace hg::engine::serialization;
using namespace hg::engine::resource;
using namespace HeliogrimApi;

namespace HeliogrimApi {
	/* Test-local component holding an ActorRef, to exercise the layout fixup table. */

	struct LinkStub : HierarchyComponent {
		using HierarchyComponent::HierarchyComponent;
		constexpr static ComponentTypeId typeId { "LinkStub"_typeId };

		ActorRef<Actor> target {};
		ComponentRef<HierarchyComponent> part {};
		u32 tag = 0u;
		bool armed = false;
	};

	class TestLinker :
		public InheritArchetype<TestLinker, Actor> {
	public:
		using this_type = TestLinker;
		constexpr static type_id typeId { "TestLinker"_typeId };

		using layout = ActorLayout<Slot<"link", LinkStub>>;

	public:
		explicit TestLinker(cref<ActorInitializer> initializer_) :
			InheritArchetype(initializer_) {}
	};
}

template <>
class hg::engine::serialization::access::Layout<HeliogrimApi::LinkStub> {
public:
	constexpr static std::array fields {
		HG_LAYOUT_RAW(HeliogrimApi::LinkStub, tag, ::hg::u32),
		HG_LAYOUT_MASK(HeliogrimApi::LinkStub, armed, 0x01u)
	};
	constexpr static auto refs = std::tuple { &HeliogrimApi::LinkStub::target, &HeliogrimApi::LinkStub::part };
};

namespace HeliogrimApi {
	/* Plain probes for the copy plan and the field hash; never archetype slots. */

	struct PlanProbe {
		u32 a;
		u32 b;
		bool c;
		u8 pad;
		u16 d;
		f32 e[3];
	};

	struct HashProbe {
		u32 a;
		u32 b;
	};

	// Same streamed field as HashProbe, at another offset (as a larger Debug member before it would do).
	struct HashProbeMoved {
		u64 debugOnly;
		u32 a;
	};

	// Same shape as HashProbe, other member name.
	struct HashProbeRenamed {
		u32 z;
	};
}

template <>
class hg::engine::serialization::access::Layout<HeliogrimApi::PlanProbe> {
public:
	constexpr static std::array fields {
		HG_LAYOUT_RAW(HeliogrimApi::PlanProbe, a, ::hg::u32),
		HG_LAYOUT_RAW(HeliogrimApi::PlanProbe, b, ::hg::u32),
		HG_LAYOUT_MASK(HeliogrimApi::PlanProbe, c, 0x01u),
		HG_LAYOUT_RAW(HeliogrimApi::PlanProbe, d, ::hg::u16),
		HG_LAYOUT_RAW(HeliogrimApi::PlanProbe, e, ::hg::f32[3])
	};
};

template <>
class hg::engine::serialization::access::Layout<HeliogrimApi::HashProbe> {
public:
	constexpr static std::array fields { HG_LAYOUT_RAW(HeliogrimApi::HashProbe, a, ::hg::u32) };
};

template <>
class hg::engine::serialization::access::Layout<HeliogrimApi::HashProbeMoved> {
public:
	constexpr static std::array fields { HG_LAYOUT_RAW(HeliogrimApi::HashProbeMoved, a, ::hg::u32) };
};

template <>
class hg::engine::serialization::access::Layout<HeliogrimApi::HashProbeRenamed> {
public:
	constexpr static std::array fields { HG_LAYOUT_RAW(HeliogrimApi::HashProbeRenamed, z, ::hg::u32) };
};

/* Structured schemes of the stub payloads. */

#define HELIOGRIM_TEST_STUB_STRUCTURE(Type_, Field_, Value_) \
	template <> \
	void access::Structure<HeliogrimApi::Type_>::serialize(ref<const HeliogrimApi::Type_> self_, mref<StructScopedSlot> record_) { \
		record_.insertSlot<Value_>(#Field_) << self_.Field_; \
	} \
	template <> \
	void access::Structure<HeliogrimApi::Type_>::hydrate(ref<const StructScopedSlot> record_, ref<HeliogrimApi::Type_> target_) { \
		record_.getSlot<Value_>(#Field_) >> target_.Field_; \
	}

HELIOGRIM_TEST_STUB_STRUCTURE(BodyStub, mass, u64)

HELIOGRIM_TEST_STUB_STRUCTURE(WheelComponentStub, radius, float)

HELIOGRIM_TEST_STUB_STRUCTURE(HubStub, bolts, u32)

HELIOGRIM_TEST_STUB_STRUCTURE(CameraStub, ticks, u32)

HELIOGRIM_TEST_STUB_STRUCTURE(BigCameraStub, ticks, u32)

HELIOGRIM_TEST_STUB_STRUCTURE(TurretStub, shells, u32)

HELIOGRIM_TEST_STUB_STRUCTURE(LinkStub, tag, u32)

#undef HELIOGRIM_TEST_STUB_STRUCTURE

template class hg::InheritArchetype<HeliogrimApi::TestLinker, hg::Actor>;

namespace HeliogrimApi {
	namespace {
		using TankInfo = LayoutInfo<TestTank::layout, TestTank>;
		using LinkerInfo = LayoutInfo<TestLinker::layout, TestLinker>;

		/* Constructed slot memory of `count_` archetype actors. */
		class Slots {
		public:
			Slots(ref<const ActorClass> class_, const u32 count_, const std::byte fill_ = std::byte {}) :
				_class(class_),
				_count(count_),
				_memory(
					static_cast<ptr<std::byte>>(
						::operator new(static_cast<size_t>(count_) * class_.desc().stride, std::align_val_t { class_.desc().align })
					)
				) {
				// Construction leaves padding as it finds it, so a fill makes padding leaks visible.
				std::memset(_memory, std::to_integer<int>(fill_), static_cast<size_t>(count_) * class_.desc().stride);
				_class.ops().construct(_memory, _count, stride(), ArchetypeInit { registry() });
			}

			Slots(ref<const Slots>) = delete;

			Slots& operator=(ref<const Slots>) = delete;

			~Slots() {
				_class.ops().destroy(_memory, _count, stride());
				::operator delete(_memory, std::align_val_t { _class.desc().align });
			}

		public:
			[[nodiscard]] u32 stride() const noexcept {
				return _class.desc().stride;
			}

			[[nodiscard]] ptr<std::byte> slot(const u32 index_) const noexcept {
				return _memory + static_cast<size_t>(index_) * stride();
			}

		private:
			[[nodiscard]] static ref<Registry> registry() {
				static Registry instance {};
				return instance;
			}

		private:
			ref<const ActorClass> _class;
			u32 _count;
			ptr<std::byte> _memory;
		};

		template <typename Type_>
		[[nodiscard]] ref<Type_> at(const ptr<std::byte> slot_, const u32 entry_) noexcept {
			return *std::launder(reinterpret_cast<ptr<Type_>>(slot_ + TankInfo::entries[entry_].offset));
		}

		[[nodiscard]] std::string_view leaf(const u32 entry_) noexcept {
			const auto path = TankInfo::slot_path(TankInfo::slot_of(entry_));
			return path.substr(path.rfind('/') + 1uLL);
		}

		void write(const ptr<std::byte> slot_, const u32 entry_, const u32 value_) {
			const auto name = leaf(entry_);
			if (name == "body") {
				at<BodyStub>(slot_, entry_).mass = value_;
			} else if (name == "wheels") {
				at<WheelComponentStub>(slot_, entry_).radius = static_cast<float>(value_);
			} else if (name == "hub") {
				at<HubStub>(slot_, entry_).bolts = value_;
			} else if (name == "camera") {
				at<CameraStub>(slot_, entry_).ticks = value_;
			} else {
				at<TurretStub>(slot_, entry_).shells = value_;
			}
		}

		[[nodiscard]] ref<Actor> actor(ref<const ActorClass> class_, const ptr<std::byte> slot_) noexcept {
			return *std::launder(reinterpret_cast<ptr<Actor>>(slot_ + class_.desc().actorOffset));
		}

		[[nodiscard]] ref<LinkStub> link(const ptr<std::byte> slot_) noexcept {
			return *std::launder(reinterpret_cast<ptr<LinkStub>>(slot_ + LinkerInfo::entries[0].offset));
		}

		[[nodiscard]] u64 read(const ptr<std::byte> slot_, const u32 entry_) {
			const auto name = leaf(entry_);
			if (name == "body") {
				return at<BodyStub>(slot_, entry_).mass;
			}
			if (name == "wheels") {
				return static_cast<u64>(at<WheelComponentStub>(slot_, entry_).radius);
			}
			if (name == "hub") {
				return at<HubStub>(slot_, entry_).bolts;
			}
			if (name == "camera") {
				return at<CameraStub>(slot_, entry_).ticks;
			}
			return at<TurretStub>(slot_, entry_).shells;
		}
	}

	class ArchetypeSerPublic : public ::testing::Test {
	protected:
		uptr<test_engine::SpawnEngine> _engine;

		void SetUp() override {
			_engine = make_uptr<test_engine::SpawnEngine>();
		}

		void TearDown() override {
			_engine.reset();
		}
	};

	TEST_F(ArchetypeSerPublic, StructuredRoundTrip) {
		ref<const ActorClass> tank = TestTank::actorClass();
		const Slots source { tank, 1u };
		for (u32 entry = 0u; entry < TankInfo::entry_count; ++entry) {
			write(source.slot(0u), entry, 10u + entry);
		}
		actor(tank, source.slot(0u)).setLabel(ActorLabel { "tank-7" });

		auto archive = BufferArchive {}; {
			auto structured = StructuredArchive { archive };
			auto root = structured.insertRootSlot().intoStruct();
			access::Structure<Actor>::serialize(actor(tank, source.slot(0u)), ::hg::move(root));
		}

		archive.seek(0); {
			auto structured = StructuredArchive { archive };
			const auto root = structured.getRootSlot().asStruct();
			const auto components = root.getRecordSlot(Symbols::Components).asStruct();

			EXPECT_TRUE(components.hasRecordSlot("body"));
			EXPECT_TRUE(components.hasRecordSlot("body/wheels/0"));
			EXPECT_TRUE(components.hasRecordSlot("body/wheels/2/hub"));
			EXPECT_TRUE(components.hasRecordSlot("body/camera"));

			auto created = hg::await(CreateActor(&tank, SerializedActor { root })).value();
			ASSERT_NE(created.get(), nullptr);

			const auto slot = reinterpret_cast<ptr<std::byte>>(created.get()) - tank.desc().actorOffset;
			EXPECT_EQ(created->guid(), actor(tank, source.slot(0u)).guid());
			EXPECT_EQ(created->label().text, "tank-7");
			for (u32 entry = 0u; entry < TankInfo::entry_count; ++entry) {
				EXPECT_EQ(read(slot, entry), 10u + entry) << TankInfo::slot_path(TankInfo::slot_of(entry));
			}
		}
	}

	TEST_F(ArchetypeSerPublic, LevelSchemeRoundTrip) {
		ref<const ActorClass> tank = TestTank::actorClass();
		const Slots source { tank, 1u };
		for (u32 entry = 0u; entry < TankInfo::entry_count; ++entry) {
			write(source.slot(0u), entry, 20u + entry);
		}

		FnSets schemes {};
		schemes.addActorScheme(TestTank::typeId, MakeArchetypeFnSet(&tank));
		const auto scheme = schemes.getActorScheme(TestTank::typeId);
		ASSERT_TRUE(scheme.has_value());

		auto archive = BufferArchive {}; {
			auto structured = StructuredArchive { archive };
			scheme->serialize(actor(tank, source.slot(0u)), structured.insertRootSlot().intoStruct());
		}

		archive.seek(0);

		auto structured = StructuredArchive { archive };
		const auto created = VolatileActor<> { scheme->deserialize(structured.getRootSlot(), nullptr) };
		ASSERT_NE(created.get(), nullptr);

		const auto slot = reinterpret_cast<ptr<std::byte>>(created.get()) - tank.desc().actorOffset;
		EXPECT_EQ(created->guid(), actor(tank, source.slot(0u)).guid());
		for (u32 entry = 0u; entry < TankInfo::entry_count; ++entry) {
			EXPECT_EQ(read(slot, entry), 20u + entry);
		}
	}

	TEST_F(ArchetypeSerPublic, HydrateSkipsMissingEntries) {
		ref<const ActorClass> tank = TestTank::actorClass();
		const Slots target { tank, 1u };
		for (u32 entry = 0u; entry < TankInfo::entry_count; ++entry) {
			write(target.slot(0u), entry, 30u + entry);
		}

		auto archive = BufferArchive {}; {
			auto structured = StructuredArchive { archive };
			auto root = structured.insertRootSlot().intoStruct();
			auto components = root.insertRecordSlot(Symbols::Components).intoStruct();
		}

		archive.seek(0);

		auto structured = StructuredArchive { archive };
		tank.ops().hydrate(target.slot(0u), structured.getRootSlot().asStruct());
		for (u32 entry = 0u; entry < TankInfo::entry_count; ++entry) {
			EXPECT_EQ(read(target.slot(0u), entry), 30u + entry);
		}
	}

	TEST_F(ArchetypeSerPublic, HydrateReportsMissingAndUnknownEntries) {
		ref<const ActorClass> tank = TestTank::actorClass();
		const Slots target { tank, 1u };

		auto archive = BufferArchive {}; {
			auto structured = StructuredArchive { archive };
			auto root = structured.insertRootSlot().intoStruct();
			auto components = root.insertRecordSlot(Symbols::Components).intoStruct();
			components.insertRecordSlot("bogus/path").intoStruct();
		}

		archive.seek(0);

		LoadContext ctx { 1uLL << 20 };
		const auto root = RootScopedSlot { archive, ScopedSlotStateFlag::eClean, &ctx };
		tank.ops().hydrate(target.slot(0u), root.asStruct());
		EXPECT_TRUE(actor(tank, target.slot(0u)).carriesUnknownData());

		engine::diag::CollectingSink sink {};
		std::ignore = ctx.finish(sink, "archetype");
		u32 missing = 0u;
		u32 unknown = 0u;
		for (const auto& item : sink.items()) {
			missing += item.code() != nullptr && item.code()->key == "missing-field";
			unknown += item.code() != nullptr && item.code()->key == "unknown-field";
		}
		EXPECT_EQ(missing, TankInfo::entry_count);
		EXPECT_EQ(unknown, 1u);
	}

	TEST_F(ArchetypeSerPublic, WrongClassRecordFails) {
		ref<const ActorClass> body = HeliogrimApi::TestBody::actorClass();
		const Slots source { body, 1u };

		auto archive = BufferArchive {}; {
			auto structured = StructuredArchive { archive };
			auto root = structured.insertRootSlot().intoStruct();
			access::Structure<Actor>::serialize(actor(body, source.slot(0u)), ::hg::move(root));
		}

		archive.seek(0); {
			auto structured = StructuredArchive { archive };
			const auto root = structured.getRootSlot().asStruct();
			const auto created = hg::await(CreateActor(&TestTank::actorClass(), SerializedActor { root })).value();
			EXPECT_EQ(created.get(), nullptr);
		}

		archive.seek(0); {
			// Rejection returns before the universe is touched, so an empty one suffices.
			auto structured = StructuredArchive { archive };
			const auto root = structured.getRootSlot().asStruct();
			const auto spawned = hg::await(SpawnActor(&TestTank::actorClass(), SerializedActor { root }, Universe {})).value();
			EXPECT_FALSE(spawned.has_value());
		}
	}

	// Without an asset database only unbound geometry can round trip.
	TEST_F(ArchetypeSerPublic, StaticGeometryActorStructuredRoundTrip) {
		ref<const ActorClass> cls = StaticGeometryActor::actorClass();
		const Slots source { cls, 1u };
		actor(cls, source.slot(0u)).setLabel(ActorLabel { "geo-1" });

		auto archive = BufferArchive {}; {
			auto structured = StructuredArchive { archive };
			access::Structure<Actor>::serialize(actor(cls, source.slot(0u)), structured.insertRootSlot().intoStruct());
		}

		archive.seek(0);

		auto structured = StructuredArchive { archive };
		const auto root = structured.getRootSlot().asStruct();
		EXPECT_TRUE(root.getRecordSlot(Symbols::Components).asStruct().hasRecordSlot("geometry"));

		auto created = hg::await(CreateActor(&cls, SerializedActor { root })).value();
		ASSERT_NE(created.get(), nullptr);
		EXPECT_EQ(created->guid(), actor(cls, source.slot(0u)).guid());
		EXPECT_EQ(created->label().text, "geo-1");
		EXPECT_FALSE(static_cast<ptr<StaticGeometryActor>>(created.get())->get<"geometry">().getStaticGeometry().isValid());
	}

	TEST_F(ArchetypeSerPublic, ComponentOffsetMaxRejected) {
		auto &registry = *_engine->getActors()->getRegistry();
		auto created = CreateActor<TestTank>();
		const auto handle = registry.handleOf(*created.get());
		constexpr auto componentId = refl::TypeId<HierarchyComponent>();
		const auto &tank = TestTank::actorClass();
		const auto valid = static_cast<std::ptrdiff_t>(tank.componentOffsets().front()) - static_cast<std::ptrdiff_t>(tank.desc().actorOffset);

		EXPECT_NE(registry.resolveComponent(handle, valid, componentId), nullptr);
		EXPECT_EQ(registry.resolveComponent(handle, std::numeric_limits<std::ptrdiff_t>::max(), componentId), nullptr);
		EXPECT_EQ(registry.resolveComponent(handle, std::numeric_limits<std::ptrdiff_t>::min(), componentId), nullptr);
		EXPECT_EQ(registry.resolveComponent(handle, -static_cast<std::ptrdiff_t>(tank.desc().actorOffset) - 1, componentId), nullptr);
		// Right offset, wrong entry type.
		EXPECT_EQ(registry.resolveComponent(handle, valid, refl::TypeId<LinkStub>()), nullptr);
	}

	TEST_F(ArchetypeSerPublic, ActorRefWrongClassResolvesNull) {
		auto &registry = *_engine->getActors()->getRegistry();
		auto created = CreateActor<TestTank>();
		const auto handle = registry.handleOf(*created.get());

		EXPECT_NE(registry.resolve(handle, refl::TypeId<TestTank>()), nullptr);
		EXPECT_NE(registry.resolve(handle, refl::TypeId<Actor>()), nullptr);
		EXPECT_EQ(registry.resolve(handle, refl::TypeId<TestLinker>()), nullptr);
	}

	TEST(ArchetypeSer, StaticGeometryActorLayoutRoundTrip) {
		ref<const ActorClass> cls = StaticGeometryActor::actorClass();
		const Slots source { cls, 2u };
		const Slots target { cls, 2u };

		Vector<std::byte> bytes {};
		cls.ops().dumpLayout(source.slot(0u), 2u, source.stride(), bytes);
		ASSERT_TRUE(cls.ops().loadLayout(target.slot(0u), 2u, target.stride(), bytes).has_value());

		auto& loaded = static_cast<ref<StaticGeometryActor>>(actor(cls, target.slot(1u)));
		EXPECT_FALSE(loaded.get<"geometry">().getStaticGeometry().isValid());
		EXPECT_TRUE(loaded.get<"geometry">().getInstanceMaterials().empty());
	}

	// Material slots from file data must not exceed the resolved geometry's mesh materials.
	TEST(ArchetypeSer, StaticGeometryLayoutRejectsSlotsBeyondMeshMaterials) {
		using StaticGeometryLayout = access::Layout<StaticGeometryComponent>;
		struct Access : AssetHandle {
			static auto guidMember() noexcept { return &Access::_guid; }
			static auto typeMember() noexcept { return &Access::_type; }
			static auto refMember() noexcept { return &Access::_ref; }
		};

		auto geometry = engine::assets::StaticGeometry {
			AssetGuid { 1uL, 2u, 3u, 4uLL }, StringView { "geometry" }, engine::assets::AssetReferenceUrl {}, engine::assets::AssetUrl {}, Vector<fs::Url> {},
			AutoArray<AssetGuid> { invalid_asset_guid, invalid_asset_guid }
		};
		StaticGeometryAssetHandle handle {};
		handle.*Access::guidMember() = geometry.getAssetGuid();
		handle.*Access::typeMember() = engine::assets::StaticGeometry::typeId;
		handle.*Access::refMember() = Some(static_cast<ref<engine::assets::Asset>>(geometry));
		ASSERT_TRUE(handle.isValid());

		ref<const ActorClass> cls = StaticGeometryActor::actorClass();
		const Slots slots { cls, 1u };
		auto& component = static_cast<ref<StaticGeometryActor>>(actor(cls, slots.slot(0u))).get<"geometry">();
		const auto setSlots = [&component](const u64 count_) {
			std::memcpy(reinterpret_cast<std::byte*>(&component) + StaticGeometryLayout::fields[0].offset, &count_, sizeof(count_));
		};

		setSlots(3uLL);
		EXPECT_FALSE(StaticGeometryLayout::acceptGeometry(component, handle));
		EXPECT_FALSE(component.getStaticGeometry().isValid());

		setSlots(2uLL);
		EXPECT_TRUE(StaticGeometryLayout::acceptGeometry(component, handle));
		EXPECT_TRUE(component.getStaticGeometry().isValid());
	}

	// A material index past the layout capacity is ignored instead of tripping dumpAssets later.
	TEST(ArchetypeSer, StaticGeometryInstanceMaterialBeyondCapacityIgnored) {
		using StaticGeometryLayout = access::Layout<StaticGeometryComponent>;
		struct Access : AssetHandle {
			static auto guidMember() noexcept { return &Access::_guid; }
			static auto typeMember() noexcept { return &Access::_type; }
			static auto refMember() noexcept { return &Access::_ref; }
		};

		auto geometry = engine::assets::StaticGeometry {
			AssetGuid { 1uL, 2u, 3u, 4uLL }, StringView { "geometry" }, engine::assets::AssetReferenceUrl {}, engine::assets::AssetUrl {},
			Vector<fs::Url> {}, AutoArray<AssetGuid>(StaticGeometryLayout::material_capacity + 4uLL, invalid_asset_guid)
		};
		StaticGeometryAssetHandle handle {};
		handle.*Access::guidMember() = geometry.getAssetGuid();
		handle.*Access::typeMember() = engine::assets::StaticGeometry::typeId;
		handle.*Access::refMember() = Some(static_cast<ref<engine::assets::Asset>>(geometry));

		ref<const ActorClass> cls = StaticGeometryActor::actorClass();
		const Slots slots { cls, 1u };
		auto& component = static_cast<ref<StaticGeometryActor>>(actor(cls, slots.slot(0u))).get<"geometry">();
		ASSERT_TRUE(component.setStaticGeometry(handle));

		GraphicsMaterialAssetHandle material {};
		component.setInstanceMaterial(static_cast<u16>(StaticGeometryLayout::material_capacity), material);
		EXPECT_TRUE(component.getInstanceMaterials().empty());

		component.setInstanceMaterial(static_cast<u16>(StaticGeometryLayout::material_capacity - 1uLL), material);
		EXPECT_EQ(component.getInstanceMaterials().size(), StaticGeometryLayout::material_capacity);
	}

	// Level.Structured.cpp hydrate looks the scheme up by the Symbols::Type key that Structure<Actor> writes.
	TEST(ArchetypeSer, LevelTypeKeyFindsGeometryScheme) {
		ref<const ActorClass> cls = StaticGeometryActor::actorClass();
		const Slots source { cls, 1u };
		EXPECT_EQ(actor(cls, source.slot(0u)).getMetaClass()->typeId(), StaticGeometryActor::typeId);

		FnSets schemes {};
		schemes.addActorScheme(StaticGeometryActor::typeId, MakeArchetypeFnSet(&cls));

		auto archive = BufferArchive {}; {
			auto structured = StructuredArchive { archive };
			access::Structure<Actor>::serialize(actor(cls, source.slot(0u)), structured.insertRootSlot().intoStruct());
		}

		archive.seek(0);

		auto structured = StructuredArchive { archive };
		type_id written {};
		structured.getRootSlot().asStruct().getSlot<u64>(Symbols::Type) >> written.data;
		EXPECT_EQ(written, StaticGeometryActor::typeId);
		EXPECT_TRUE(schemes.getActorScheme(written).has_value());
	}

	TEST(ArchetypeSer, LayoutRoundTrip) {
		constexpr u32 count = 100u;
		ref<const ActorClass> tank = TestTank::actorClass();
		const Slots source { tank, count };
		const Slots target { tank, count };
		for (u32 index = 0u; index < count; ++index) {
			for (u32 entry = 0u; entry < TankInfo::entry_count; ++entry) {
				write(source.slot(index), entry, index * 16u + entry);
			}
		}

		Vector<std::byte> bytes {};
		tank.ops().dumpLayout(source.slot(0u), count, source.stride(), bytes);
		ASSERT_TRUE(tank.ops().loadLayout(target.slot(0u), count, target.stride(), bytes).has_value());

		for (u32 index = 0u; index < count; ++index) {
			for (u32 entry = 0u; entry < TankInfo::entry_count; ++entry) {
				EXPECT_EQ(read(target.slot(index), entry), index * 16u + entry);
			}
		}
	}

	TEST(ArchetypeSer, LayoutHashMismatchFails) {
		ref<const ActorClass> tank = TestTank::actorClass();
		ref<const ActorClass> body = HeliogrimApi::TestBody::actorClass();
		const Slots bodies { body, 1u };
		const Slots tanks { tank, 1u };

		Vector<std::byte> bytes {};
		body.ops().dumpLayout(bodies.slot(0u), 1u, bodies.stride(), bytes);
		EXPECT_FALSE(tank.ops().loadLayout(tanks.slot(0u), 1u, tanks.stride(), bytes).has_value());

		// Same archetype, corrupted hash.
		bytes.clear();
		tank.ops().dumpLayout(tanks.slot(0u), 1u, tanks.stride(), bytes);
		bytes[0] ^= std::byte { 0xFF };
		EXPECT_FALSE(tank.ops().loadLayout(tanks.slot(0u), 1u, tanks.stride(), bytes).has_value());
	}

	[[nodiscard]] engine::acs::Handle testHandle(const u32 index_) {
		using namespace engine::acs;
		return Handle::make(SpaceId { 1u }, HandleGeneration { 0u }, HandleIndex { index_ });
	}

	TEST(ArchetypeSer, LayoutFixupRestoresRefs) {
		using access_t = archetype_detail::RefAccess;
		constexpr u32 count = 4u;
		ref<const ActorClass> linker = TestLinker::actorClass();
		const Slots source { linker, count };
		const Slots target { linker, count };
		for (u32 index = 0u; index < count; ++index) {
			access_t::handle(link(source.slot(index)).target) = testHandle(0x1000u + index);
			access_t::handle(link(source.slot(index)).part) = testHandle(0x2000u + index);
			access_t::offset(link(source.slot(index)).part) = 48 + static_cast<std::ptrdiff_t>(index);
			link(source.slot(index)).tag = 7u * index;
			access_t::epoch(link(target.slot(index)).target) = 5uLL;
			access_t::epoch(link(target.slot(index)).part) = 5uLL;
		}

		Vector<std::byte> bytes {};
		linker.ops().dumpLayout(source.slot(0u), count, source.stride(), bytes);
		ASSERT_TRUE(linker.ops().loadLayout(target.slot(0u), count, target.stride(), bytes).has_value());

		for (u32 index = 0u; index < count; ++index) {
			EXPECT_EQ(link(target.slot(index)).target.handle(), testHandle(0x1000u + index));
			EXPECT_EQ(access_t::epoch(link(target.slot(index)).target), ~0uLL);
			EXPECT_EQ(access_t::cached(link(target.slot(index)).target), nullptr);
			EXPECT_EQ(link(target.slot(index)).part.handle(), testHandle(0x2000u + index));
			EXPECT_EQ(link(target.slot(index)).part.offset(), 48 + static_cast<std::ptrdiff_t>(index));
			EXPECT_EQ(access_t::epoch(link(target.slot(index)).part), ~0uLL);
			EXPECT_EQ(access_t::cached(link(target.slot(index)).part), nullptr);
			EXPECT_EQ(link(target.slot(index)).tag, 7u * index);
		}
	}

	TEST(ArchetypeSer, FailedLoadLeavesTargetUntouched) {
		ref<const ActorClass> tank = TestTank::actorClass();
		const Slots source { tank, 2u };
		const Slots target { tank, 2u };
		for (u32 index = 0u; index < 2u; ++index) {
			for (u32 entry = 0u; entry < TankInfo::entry_count; ++entry) {
				write(source.slot(index), entry, 1u + entry);
				write(target.slot(index), entry, 900u + entry);
			}
		}

		Vector<std::byte> bytes {};
		tank.ops().dumpLayout(source.slot(0u), 2u, source.stride(), bytes);
		bytes.pop_back();
		EXPECT_FALSE(tank.ops().loadLayout(target.slot(0u), 2u, target.stride(), bytes).has_value());
		for (u32 index = 0u; index < 2u; ++index) {
			for (u32 entry = 0u; entry < TankInfo::entry_count; ++entry) {
				EXPECT_EQ(read(target.slot(index), entry), 900u + entry);
			}
		}

		// Corrupt fixup table: payload is valid, but the last fixup names the wrong slot.
		ref<const ActorClass> linker = TestLinker::actorClass();
		const Slots links { linker, 2u };
		const Slots linkTarget { linker, 2u };
		archetype_detail::RefAccess::handle(link(linkTarget.slot(1u)).target) = testHandle(42u);
		link(linkTarget.slot(1u)).tag = 99u;

		bytes.clear();
		linker.ops().dumpLayout(links.slot(0u), 2u, links.stride(), bytes);
		bytes[bytes.size() - sizeof(archetype_detail::LayoutFixup)] ^= std::byte { 0x01 };
		EXPECT_FALSE(linker.ops().loadLayout(linkTarget.slot(0u), 2u, linkTarget.stride(), bytes).has_value());
		EXPECT_EQ(link(linkTarget.slot(0u)).tag, 0u);
		EXPECT_EQ(link(linkTarget.slot(1u)).tag, 99u);
		EXPECT_EQ(link(linkTarget.slot(1u)).target.handle(), testHandle(42u));
	}

	/* Layout contract (L1-L8). */

	enum class ProbeEnum : u8 { eA };
	static_assert(not access::raw_copyable_leaf<bool>);
	static_assert(not access::raw_copyable_leaf<ProbeEnum>);
	static_assert(not access::raw_copyable_leaf<ptr<int>>);
	static_assert(access::raw_copyable_leaf<float[3]>);
	static_assert(access::raw_copyable_leaf<u16>);

	TEST(ArchetypeSer, CopyPlanMergesAdjacentRuns) {
		using plan = archetype_detail::copy_plan<PlanProbe>;
		static_assert(plan::stream_size == 23uLL);
		static_assert(plan::runs.size() == 2uLL);
		static_assert(plan::checks.size() == 1uLL);
		EXPECT_EQ(plan::runs[0].stream, 0uLL);
		EXPECT_EQ(plan::runs[0].member, 0uLL);
		EXPECT_EQ(plan::runs[0].size, 8uLL);
		EXPECT_EQ(plan::runs[1].stream, 9uLL);
		EXPECT_EQ(plan::runs[1].member, 10uLL);
		EXPECT_EQ(plan::runs[1].size, 14uLL);
		EXPECT_EQ(plan::checks[0].stream, 8uLL);
		EXPECT_EQ(plan::checks[0].member, 8uLL);
		EXPECT_EQ(plan::checks[0].mask, 0x01u);
	}

	TEST(ArchetypeSer, LayoutHashChangesWithFields) {
		EXPECT_NE(archetype_detail::field_hash<PlanProbe>(), archetype_detail::field_hash<HashProbe>());
		EXPECT_NE(archetype_detail::field_hash<HashProbe>(), archetype_detail::field_hash<HashProbeRenamed>());
	}

	TEST(ArchetypeSer, LayoutHashIgnoresMemberOffsets) {
		static_assert(offsetof(HashProbe, a) != offsetof(HashProbeMoved, a));
		EXPECT_EQ(archetype_detail::field_hash<HashProbe>(), archetype_detail::field_hash<HashProbeMoved>());
	}

	TEST(ArchetypeSer, BoolFieldTwoRejectedBeforeWrite) {
		ref<const ActorClass> linker = TestLinker::actorClass();
		const Slots source { linker, 2u };
		const Slots target { linker, 2u };
		link(source.slot(0u)).tag = 1u;
		link(source.slot(1u)).tag = 2u;
		link(source.slot(1u)).armed = true;
		link(target.slot(0u)).tag = 77u;

		Vector<std::byte> bytes {};
		linker.ops().dumpLayout(source.slot(0u), 2u, source.stride(), bytes);
		// Header 24, then per slot tag (4) and armed (1): slot 1 armed sits at 24 + 5 + 4.
		ASSERT_EQ(bytes[33], std::byte { 1 });
		bytes[33] = std::byte { 2 };

		Vector<std::byte> before(target.slot(0u), target.slot(0u) + 2uLL * target.stride());
		const auto result = linker.ops().loadLayout(target.slot(0u), 2u, target.stride(), bytes);
		ASSERT_FALSE(result.has_value());
		EXPECT_EQ(result.error().kind, LayoutError::Kind::eInvalidValue);
		EXPECT_EQ(result.error().offset, 33uLL);
		EXPECT_EQ(std::memcmp(before.data(), target.slot(0u), before.size()), 0);
	}

	TEST(ArchetypeSer, LayoutMismatchKindAndOffset) {
		ref<const ActorClass> linker = TestLinker::actorClass();
		const Slots source { linker, 2u };
		const Slots target { linker, 2u };
		link(source.slot(0u)).tag = 1u;
		link(target.slot(0u)).tag = 77u;
		link(target.slot(1u)).tag = 78u;

		Vector<std::byte> valid {};
		linker.ops().dumpLayout(source.slot(0u), 2u, source.stride(), valid);

		const auto expectMismatch = [&](cref<Vector<std::byte>> bytes_, const u32 count_, const u64 offset_) {
			const auto result = linker.ops().loadLayout(target.slot(0u), count_, target.stride(), bytes_);
			ASSERT_FALSE(result.has_value());
			EXPECT_EQ(result.error().kind, LayoutError::Kind::eMismatch);
			EXPECT_EQ(result.error().offset, offset_);
			EXPECT_EQ(link(target.slot(0u)).tag, 77u);
			EXPECT_EQ(link(target.slot(1u)).tag, 78u);
		};

		// Shorter than the header.
		expectMismatch(Vector<std::byte>(valid.begin(), valid.begin() + sizeof(archetype_detail::LayoutHeader) - 1uLL), 2u, 0uLL);
		// Header names another slot count.
		expectMismatch(valid, 1u, 0uLL);
		// Header fits, the body is one byte short.
		expectMismatch(Vector<std::byte>(valid.begin(), valid.end() - 1), 2u, sizeof(archetype_detail::LayoutHeader));
		// Second fixup names the wrong slot.
		auto fixup = valid;
		const u64 lastFixup = fixup.size() - sizeof(archetype_detail::LayoutFixup);
		fixup[lastFixup] ^= std::byte { 0x01 };
		expectMismatch(fixup, 2u, lastFixup);

		// The untouched stream still loads.
		ASSERT_TRUE(linker.ops().loadLayout(target.slot(0u), 2u, target.stride(), valid).has_value());
		EXPECT_EQ(link(target.slot(0u)).tag, 1u);
	}

	// Padding and unmasked bits differ between the two sources and must never reach the stream.
	TEST(ArchetypeSer, DumpIsDeterministic) {
		ref<const ActorClass> tank = TestTank::actorClass();
		const Slots first { tank, 3u, std::byte { 0xAA } };
		const Slots second { tank, 3u, std::byte { 0x55 } };
		for (u32 index = 0u; index < 3u; ++index) {
			for (u32 entry = 0u; entry < TankInfo::entry_count; ++entry) {
				write(first.slot(index), entry, index + entry);
				write(second.slot(index), entry, index + entry);
			}
		}

		Vector<std::byte> a {};
		Vector<std::byte> b {};
		tank.ops().dumpLayout(first.slot(0u), 3u, first.stride(), a);
		tank.ops().dumpLayout(second.slot(0u), 3u, second.stride(), b);
		EXPECT_EQ(a, b);
	}

	TEST(ArchetypeSer, DumpIgnoresUnmaskedBits) {
		ref<const ActorClass> linker = TestLinker::actorClass();
		const Slots first { linker, 2u, std::byte { 0xAA } };
		const Slots second { linker, 2u, std::byte { 0x55 } };
		for (u32 index = 0u; index < 2u; ++index) {
			for (const auto* slots : { &first, &second }) {
				link(slots->slot(index)).tag = index + 1u;
			}
			// Same used bit (0x01), different garbage in the bits outside the mask.
			reinterpret_cast<std::byte&>(link(first.slot(index)).armed) = std::byte { 0xA1 };
			reinterpret_cast<std::byte&>(link(second.slot(index)).armed) = std::byte { 0x5D };
		}

		Vector<std::byte> a {};
		Vector<std::byte> b {};
		linker.ops().dumpLayout(first.slot(0u), 2u, first.stride(), a);
		linker.ops().dumpLayout(second.slot(0u), 2u, second.stride(), b);
		EXPECT_EQ(a, b);
	}

	TEST_F(ArchetypeSerPublic, RejectedLoadDoesNotResolve) {
		ref<const ActorClass> cls = StaticGeometryActor::actorClass();
		const Slots source { cls, 1u };
		const Slots target { cls, 1u };

		Vector<std::byte> bytes {};
		cls.ops().dumpLayout(source.slot(0u), 1u, source.stride(), bytes);
		// Material count over capacity: rejected by the check hook, after the header and size fit.
		const u64 tooMany = 17uLL;
		std::memcpy(bytes.data() + sizeof(hg::archetype_detail::LayoutHeader), &tooMany, sizeof(tooMany));
		for (size_t index = 32uLL; index < 48uLL; ++index) {
			bytes[index] = std::byte { 0x5A };
		}

		Vector<std::byte> before(target.slot(0u), target.slot(0u) + target.stride());
		ASSERT_FALSE(cls.ops().loadLayout(target.slot(0u), 1u, target.stride(), bytes).has_value());

		LoadContext ctx { 1uLL << 20 };
		cls.ops().resolveLayout(target.slot(0u), 1u, target.stride(), bytes, ctx);
		EXPECT_EQ(std::memcmp(before.data(), target.slot(0u), before.size()), 0);

		engine::diag::CollectingSink sink {};
		std::ignore = ctx.finish(sink, "layout");
		EXPECT_TRUE(sink.items().empty());
	}

	TEST_F(ArchetypeSerPublic, UnresolvedAssetGuidWarns) {
		ref<const ActorClass> cls = StaticGeometryActor::actorClass();
		const Slots source { cls, 1u };
		const Slots target { cls, 1u };

		Vector<std::byte> bytes {};
		cls.ops().dumpLayout(source.slot(0u), 1u, source.stride(), bytes);
		// The geometry guid follows the header and the u64 material count; a nonzero guid no database knows.
		for (size_t index = 32uLL; index < 48uLL; ++index) {
			bytes[index] = std::byte { 0x5A };
		}
		ASSERT_TRUE(cls.ops().loadLayout(target.slot(0u), 1u, target.stride(), bytes).has_value());

		LoadContext ctx { 1uLL << 20 };
		cls.ops().resolveLayout(target.slot(0u), 1u, target.stride(), bytes, ctx);

		engine::diag::CollectingSink sink {};
		std::ignore = ctx.finish(sink, "layout");
		bool unresolved = false;
		for (const auto& item : sink.items()) {
			unresolved = unresolved || (item.code() != nullptr && item.code()->key == "unresolved-reference");
		}
		EXPECT_TRUE(unresolved);
		EXPECT_FALSE(static_cast<ref<StaticGeometryActor>>(actor(cls, target.slot(0u))).get<"geometry">().getStaticGeometry().isValid());
	}
}

namespace HeliogrimApi {
	TEST_F(ArchetypeSerPublic, SecondRootComponentRejected) {
		// The kept root is pooled so the registry owns it; the rejected one is plain heap and dropped by its own deleter.
		constexpr ComponentTypeId bodyType { 0x7E57'0000'0000'00A2uLL };
		_engine->_serialization->addComponentScheme(
			bodyType,
			ComponentFnSet {
				.serialize = [](cref<LogicComponent>, mref<StructScopedSlot>) {},
				.deserialize = [](cref<RecordScopedSlot>, nmpt<engine::acs::Registry> allocator_, ref<Actor> owner_, nmpt<HierarchyComponent> parent_) {
					return VolatileComponent<> { ComponentScheme<BodyStub>::instantiate<void>(allocator_, ComponentGuid { generate_component_guid() }, owner_, parent_).release() };
				}
			}
		);

		constexpr ComponentTypeId rootType { 0x7E57'0000'0000'00A1uLL };
		_engine->_serialization->addComponentScheme(
			rootType,
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
		_engine->_serialization->addActorScheme<Actor>();

		auto archive = BufferArchive {}; {
			auto structured = StructuredArchive { archive };
			auto root = structured.insertRootSlot().intoStruct();
			insert_guid_slot(root, ActorGuid { generate_actor_guid() });
			insert_typeId_slot(root, Actor::typeId);
			root.insertSlot<String>("label"sv) << String { "two-roots" };
			auto components = root.insertRecordSlot(Symbols::Components).intoSeq();
			for (const auto type : { bodyType, rootType }) {
				auto record = components.addRecordSlot().intoStruct();
				insert_typeId_slot(record, type);
				record.insertSlot<s8>(Symbols::Parent) << s8 { -1 };
			}
		}

		archive.seek(0);

		auto structured = StructuredArchive { archive };
		const auto scheme = _engine->_serialization->getActorScheme(Actor::typeId);
		ASSERT_TRUE(scheme.has_value());
		const auto actor = scheme->deserialize(structured.getRootSlot(), _engine->getActors()->getRegistry());

		ASSERT_NE(actor, nullptr);
		// Context-less: rejection is reported through LoadContext only (LoadBoundary.SecondRootComponentRejected).
		EXPECT_TRUE(actor->getRootComponent().has_value());
		EXPECT_EQ(actor->getComponents().size(), 1uLL);
	}
}
