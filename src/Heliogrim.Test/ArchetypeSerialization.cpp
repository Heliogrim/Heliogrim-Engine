#include "pch.h"

#include <cstddef>
#include <new>
#include <span>
#include <string_view>
#include <Engine.ACS/Registry.hpp>
#include <Engine.ACS/Archetype/LayoutInfo.hpp>
#include <Engine.Common/Collection/Vector.hpp>
#include <Engine.Resource.Archive/BufferArchive.hpp>
#include <Engine.Serialization/Access/Structure.hpp>
#include <Engine.Serialization/Archive/StructuredArchive.hpp>
#include <Engine.Serialization/Structure/FloatScopedSlot.hpp>
#include <Engine.Serialization/Structure/IntegralScopedSlot.hpp>
#include <Engine.Serialization/Structure/RecordScopedSlot.hpp>
#include <Engine.Serialization/Structure/StructScopedSlot.hpp>
#include <Engine.Serialization.Structures/Symbols.hpp>
#include <Heliogrim/Actor/ActorRef.hpp>
#include <Heliogrim/Actor/Archetype/ActorClass.hpp>
#include <Heliogrim/Actor/Archetype/ArchetypeDefinition.inl.hpp>
#include <Heliogrim/Component/ComponentRef.hpp>
#include <Heliogrim/Universe/Universe.hpp>

#include "SpawnEngine.hpp"
#include "TestArchetypes.hpp"

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
	constexpr static ::hg::u64 version = 1uLL;
	constexpr static ::hg::u64 size = sizeof(HeliogrimApi::LinkStub::tag);
	constexpr static auto refs = std::tuple { &HeliogrimApi::LinkStub::target, &HeliogrimApi::LinkStub::part };

public:
	static void dump(::hg::ref<const HeliogrimApi::LinkStub> self_, ::hg::ref<::hg::Vector<std::byte>> out_) {
		::hg::archetype_detail::put_bytes(out_, self_.tag);
	}

	static void load(std::span<const std::byte> in_, ::hg::ref<HeliogrimApi::LinkStub> target_) {
		std::ignore = ::hg::archetype_detail::take_bytes(in_, target_.tag);
	}
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
			Slots(ref<const ActorClass> class_, const u32 count_) :
				_class(class_),
				_count(count_),
				_memory(
					static_cast<ptr<std::byte>>(
						::operator new(static_cast<size_t>(count_) * class_.desc().stride, std::align_val_t { class_.desc().align })
					)
				) {
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

		auto archive = BufferArchive {};
		{
			auto structured = StructuredArchive { archive };
			auto root = structured.insertRootSlot().intoStruct();
			access::Structure<Actor>::serialize(actor(tank, source.slot(0u)), ::hg::move(root));
		}

		archive.seek(0);

		{
			auto structured = StructuredArchive { archive };
			const auto root = structured.getRootSlot().asStruct();
			const auto components = root.getRecordSlot(Symbols::Components).asStruct();

			EXPECT_TRUE(components.hasRecordSlot("body"));
			EXPECT_TRUE(components.hasRecordSlot("body/wheels/0"));
			EXPECT_TRUE(components.hasRecordSlot("body/wheels/2/hub"));
			EXPECT_TRUE(components.hasRecordSlot("body/camera"));

			auto created = CreateActor(&tank, SerializedActor { root }).get();
			ASSERT_NE(created.get(), nullptr);

			const auto slot = reinterpret_cast<ptr<std::byte>>(created.get()) - tank.desc().actorOffset;
			EXPECT_EQ(created->guid(), actor(tank, source.slot(0u)).guid());
			EXPECT_EQ(created->label().text, "tank-7");
			for (u32 entry = 0u; entry < TankInfo::entry_count; ++entry) {
				EXPECT_EQ(read(slot, entry), 10u + entry) << TankInfo::slot_path(TankInfo::slot_of(entry));
			}
		}
	}

	TEST_F(ArchetypeSerPublic, WrongClassRecordFails) {
		ref<const ActorClass> body = HeliogrimApi::TestBody::actorClass();
		const Slots source { body, 1u };

		auto archive = BufferArchive {};
		{
			auto structured = StructuredArchive { archive };
			auto root = structured.insertRootSlot().intoStruct();
			access::Structure<Actor>::serialize(actor(body, source.slot(0u)), ::hg::move(root));
		}

		archive.seek(0);

		{
			auto structured = StructuredArchive { archive };
			const auto root = structured.getRootSlot().asStruct();
			const auto created = CreateActor(&TestTank::actorClass(), SerializedActor { root }).get();
			EXPECT_EQ(created.get(), nullptr);
		}

		archive.seek(0);

		{
			// Rejection returns before the universe is touched, so an empty one suffices.
			auto structured = StructuredArchive { archive };
			const auto root = structured.getRootSlot().asStruct();
			const auto spawned = SpawnActor(&TestTank::actorClass(), SerializedActor { root }, Universe {}).get();
			EXPECT_FALSE(spawned.has_value());
		}
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
		ASSERT_TRUE(tank.ops().loadLayout(target.slot(0u), count, target.stride(), bytes));

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
		EXPECT_FALSE(tank.ops().loadLayout(tanks.slot(0u), 1u, tanks.stride(), bytes));

		// Same archetype, corrupted hash.
		bytes.clear();
		tank.ops().dumpLayout(tanks.slot(0u), 1u, tanks.stride(), bytes);
		bytes[0] ^= std::byte { 0xFF };
		EXPECT_FALSE(tank.ops().loadLayout(tanks.slot(0u), 1u, tanks.stride(), bytes));
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
		ASSERT_TRUE(linker.ops().loadLayout(target.slot(0u), count, target.stride(), bytes));

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
		EXPECT_FALSE(tank.ops().loadLayout(target.slot(0u), 2u, target.stride(), bytes));
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
		EXPECT_FALSE(linker.ops().loadLayout(linkTarget.slot(0u), 2u, linkTarget.stride(), bytes));
		EXPECT_EQ(link(linkTarget.slot(0u)).tag, 0u);
		EXPECT_EQ(link(linkTarget.slot(1u)).tag, 99u);
		EXPECT_EQ(link(linkTarget.slot(1u)).target.handle(), testHandle(42u));
	}
}
