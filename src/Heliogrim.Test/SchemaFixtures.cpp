#include "pch.h"

/**/

#include <cstdlib>
#include <filesystem>
#include <format>
#include <fstream>
#include <functional>
#include <iterator>
#include <new>

#include <Engine.ACS/Registry.hpp>
#include <Engine.Assets.Type/AssetLoad.hpp>
#include <Engine.Assets.Type/Geometry/StaticGeometry.hpp>
#include <Engine.Assets.Type/Material/GraphicsMaterial.hpp>
#include <Engine.Assets.Type/Material/GraphicsMaterialPrototype.hpp>
#include <Engine.Assets.Type/Texture/FontAsset.hpp>
#include <Engine.Assets.Type/Texture/ImageAsset.hpp>
#include <Engine.Assets.Type/Texture/TextureAsset.hpp>
#include <Engine.Assets.Type/Universe/LevelAsset.hpp>
#include <Engine.Common/Collection/Vector.hpp>
#include <Engine.Diagnostics/Sink.hpp>
#include <Engine.Level/Level.hpp>
#include <Engine.Resource.Archive/BufferArchive.hpp>
#include <Engine.Serialization/SerializationModule.hpp>
#include <Engine.Serialization/Access/Structure.hpp>
#include <Engine.Serialization/Archive/StructuredArchive.hpp>
#include <Engine.Serialization/Load/LoadContext.hpp>
#include <Engine.Serialization/Scheme/ActorScheme.hpp>
#include <Engine.Serialization/Scheme/ComponentScheme.hpp>
#include <Engine.Serialization/Structure/RootScopedSlot.hpp>
#include <Engine.Serialization/Structure/StructScopedSlot.hpp>
#include <Heliogrim/Actor/Actor.hpp>
#include <Heliogrim/Actor/Archetype/ActorClass.hpp>
#include <Heliogrim/Actor/Archetype/ArchetypeFnSet.hpp>
#include <Heliogrim/Actor/Camera/CameraActor.hpp>
#include <Heliogrim/Actor/Geometry/BoxShapeGeometryActor.hpp>
#include <Heliogrim/Actor/Geometry/SphereShapeGeometryActor.hpp>
#include <Heliogrim/Actor/Geometry/StaticGeometryActor.hpp>
#include <Heliogrim/Actor/Light/DirectionalLightActor.hpp>
#include <Heliogrim/Actor/Light/PointLightActor.hpp>
#include <Heliogrim/Component/HierarchyComponent.hpp>
#include <Heliogrim/Component/Scene/Camera/CameraComponent.hpp>
#include <Heliogrim/Component/Scene/Geometry/StaticGeometryComponent.hpp>
#include <Heliogrim/Component/Scene/Light/DirectionalLightComponent.hpp>
#include <Heliogrim/Component/Scene/Light/PointLightComponent.hpp>
#include <Heliogrim/Component/Scene/Light/SpotLightComponent.hpp>
#include <Heliogrim/Component/Scene/Sky/SkyboxComponent.hpp>

#include "SpawnEngine.hpp"
#include "TestArchetypes.hpp"

using namespace hg::engine::serialization;
using namespace hg::engine::resource;
using namespace hg::engine::assets;
using namespace hg::engine::core;
using hg::engine::Engine;
using namespace hg;

namespace HeliogrimApi::schema_fixtures {
	namespace fs = std::filesystem;

	using Bytes = Vector<std::byte>;

	struct Loaded {
		bool ok = false;
		bool errors = false;
		String diagnostics {};
		Bytes resaved {};
	};

	/* One fixture type: `write` builds the deterministic sample, `reload` loads given bytes and saves them again. */
	struct Case {
		String name;
		std::function<void(ref<BufferArchive>)> write;
		std::function<Loaded(cref<Bytes>)> reload;
	};

	[[nodiscard]] fs::path fixtureDir() {
		return fs::path { HG_FIXTURE_DIR };
	}

	[[nodiscard]] fs::path fixturePath(StringView type_) {
		return fixtureDir() / fs::path { type_ } / "r0.bin";
	}

	[[nodiscard]] bool writeEnabled() {
		const auto* const flag = std::getenv("HG_WRITE_FIXTURES");
		return flag != nullptr && StringView { flag } == "1";
	}

	[[nodiscard]] Opt<Bytes> readFile(cref<fs::path> path_) {
		std::ifstream in { path_, std::ios::binary };
		if (not in) {
			return None;
		}
		const std::string raw { std::istreambuf_iterator<char> { in }, std::istreambuf_iterator<char> {} };
		Bytes out(raw.size());
		std::memcpy(out.data(), raw.data(), raw.size());
		return Some(::hg::move(out));
	}

	void writeFile(cref<fs::path> path_, std::span<const std::byte> bytes_) {
		fs::create_directories(path_.parent_path());
		std::ofstream out { path_, std::ios::binary };
		out.write(reinterpret_cast<const char*>(bytes_.data()), static_cast<std::streamsize>(bytes_.size()));
	}

	[[nodiscard]] Bytes drain(ref<BufferArchive> archive_) {
		Bytes out(static_cast<size_t>(archive_.totalSize()));
		archive_.seek(0);
		archive_.serializeBytes(out.data(), out.size(), ArchiveStreamMode::eLoad);
		return out;
	}

	void fill(ref<BufferArchive> archive_, cref<Bytes> bytes_) {
		auto copy = bytes_;
		archive_.serializeBytes(copy.data(), copy.size(), ArchiveStreamMode::eStore);
		archive_.seek(0);
	}

	[[nodiscard]] String describe(ref<const engine::diag::CollectingSink> sink_) {
		String out {};
		for (const auto& item : sink_.items()) {
			out += std::format("[{}] {}\n", item.code() != nullptr ? String { item.code()->key } : String { "?" }, item.message());
		}
		return out;
	}

	/* Runs `hydrate_` on a context-bound root over `bytes_`; the load passes when it reports no error. */
	template <typename Fn_>
	[[nodiscard]] Loaded loadWith(cref<Bytes> bytes_, StringView url_, Fn_&& hydrate_) {
		auto archive = BufferArchive {};
		fill(archive, bytes_);
		engine::diag::CollectingSink sink {};
		auto ctx = LoadContext { static_cast<u64>(archive.totalSize()), String { url_ } };
		bool hydrated = false;
		{
			const auto root = RootScopedSlot { archive, ScopedSlotStateFlag::eClean, &ctx };
			hydrated = hydrate_(root, ctx);
		}
		auto finished = ctx.finish(sink, std::format("while loading fixture '{}'", url_));
		if (not finished.has_value()) {
			for (auto& diagnostic : finished.error()) {
				sink.emit(::hg::move(diagnostic));
			}
		}
		return Loaded { .ok = hydrated && finished.has_value() && not sink.hasErrors(), .errors = sink.hasErrors(), .diagnostics = describe(sink), .resaved = {} };
	}

	template <typename Fn_>
	[[nodiscard]] Bytes saveWith(Fn_&& serialize_) {
		auto archive = BufferArchive {};
		{
			auto structured = StructuredArchive { archive };
			serialize_(structured.insertRootSlot().intoStruct());
		}
		return drain(archive);
	}

	/**/

	[[nodiscard]] ActorGuid sampleGuid(const u8 seed_) {
		return ActorGuid { Guid { 0x5EED0000u + seed_, 1u, 2u, 3uLL } };
	}

	ptr<engine::SerializationModule> activeSerial = nullptr;

	[[nodiscard]] cref<engine::SerializationModule> serial() {
		return *activeSerial;
	}

	template <class ActorType_>
	[[nodiscard]] VolatileActor<> sampleActor(const u8 seed_) {
		if constexpr (requires { ActorType_::actorClass(); }) {
			auto actor = CreateActor<ActorType_>();
			actor->unsafe_set_guid(sampleGuid(seed_));
			return VolatileActor<> { ::hg::move(actor) };
		} else {
			// CreateActor by meta class aborts headless; the scheme path builds plain actors directly.
			ptr<Actor> actor = ActorScheme<ActorType_>::template instantiate<void>(
				Engine::getEngine()->getActors()->getRegistry(),
				sampleGuid(seed_)
			);
			return VolatileActor<> { ::hg::move(actor) };
		}
	}

	void saveActor(cref<Actor> actor_, const type_id typeId_, mref<StructScopedSlot> slot_) {
		const auto scheme = serial().getActorScheme(typeId_);
		::hg::assertrt(scheme.has_value());
		scheme->serialize(actor_, ::hg::move(slot_));
	}

	template <class ActorType_>
	[[nodiscard]] Case actorCase(StringView name_, const u8 seed_) {
		return Case {
			.name = String { name_ },
			.write = [seed_](ref<BufferArchive> archive_) {
				auto actor = sampleActor<ActorType_>(seed_);
				auto structured = StructuredArchive { archive_ };
				saveActor(*actor, ActorType_::typeId, structured.insertRootSlot().intoStruct());
			},
			.reload = [name = String { name_ }](cref<Bytes> bytes_) {
				Opt<VolatileActor<>> loaded {};
				auto result = loadWith(
					bytes_,
					name,
					[&](cref<RootScopedSlot> root_, ref<LoadContext>) {
						const auto scheme = serial().getActorScheme(ActorType_::typeId);
						if (not scheme.has_value()) {
							return false;
						}
						loaded.emplace(scheme->deserialize(root_, Engine::getEngine()->getActors()->getRegistry()));
						return loaded->get() != nullptr;
					}
				);
				if (result.ok) {
					result.resaved = saveWith([&](mref<StructScopedSlot> slot_) { saveActor(**loaded, ActorType_::typeId, ::hg::move(slot_)); });
				}
				return result;
			}
		};
	}

	template <class ComponentType_>
	[[nodiscard]] uptr<ComponentType_> sampleComponent() {
		return make_uptr<ComponentType_>(ComponentGuid {}, CachedActorPointer {}, nullptr);
	}

	template <class ComponentType_>
	[[nodiscard]] Case componentCase(StringView name_) {
		return Case {
			.name = String { name_ },
			.write = [](ref<BufferArchive> archive_) {
				const auto component = sampleComponent<ComponentType_>();
				auto structured = StructuredArchive { archive_ };
				access::Structure<ComponentType_>::serialize(*component, structured.insertRootSlot().intoStruct());
			},
			.reload = [name = String { name_ }](cref<Bytes> bytes_) {
				const auto loaded = sampleComponent<ComponentType_>();
				auto result = loadWith(
					bytes_,
					name,
					[&](cref<RootScopedSlot> root_, ref<LoadContext> ctx_) {
						const auto slot = root_.asStruct();
						access::Structure<ComponentType_>::hydrate(slot, *loaded);
						return not ctx_.failed();
					}
				);
				if (result.ok) {
					result.resaved = saveWith([&](mref<StructScopedSlot> slot_) { access::Structure<ComponentType_>::serialize(*loaded, ::hg::move(slot_)); });
				}
				return result;
			}
		};
	}

	template <class AssetType_>
	[[nodiscard]] Case assetCase(StringView name_, std::function<uptr<AssetType_>()> make_) {
		return Case {
			.name = String { name_ },
			.write = [make_](ref<BufferArchive> archive_) {
				const auto asset = make_();
				auto structured = StructuredArchive { archive_ };
				access::Structure<AssetType_>::serialize(*asset, structured.insertRootSlot().intoStruct());
			},
			.reload = [make_, name = String { name_ }](cref<Bytes> bytes_) {
				const auto loaded = make_();
				auto result = loadWith(
					bytes_,
					name,
					[&](cref<RootScopedSlot> root_, ref<LoadContext> ctx_) {
						return hydrateArchivedAsset(serial().migrations(), root_.asStruct(), ctx_, *loaded);
					}
				);
				if (result.ok) {
					result.resaved = saveWith([&](mref<StructScopedSlot> slot_) { access::Structure<AssetType_>::serialize(*loaded, ::hg::move(slot_)); });
				}
				return result;
			}
		};
	}

	[[nodiscard]] Case levelCase(StringView name_, const bool withActors_) {
		return Case {
			.name = String { name_ },
			.write = [withActors_](ref<BufferArchive> archive_) {
				auto level = make_root_like_level();
				if (withActors_) {
					std::ignore = level->addActor(sampleActor<BoxShapeGeometryActor>(0x21u));
					std::ignore = level->addActor(sampleActor<SphereShapeGeometryActor>(0x22u));
					std::ignore = level->addActor(sampleActor<StaticGeometryActor>(0x23u));
				}
				auto structured = StructuredArchive { archive_ };
				access::Structure<Level>::serialize(*level, structured.insertRootSlot().intoStruct());
			},
			.reload = [name = String { name_ }](cref<Bytes> bytes_) {
				auto archive = BufferArchive {};
				fill(archive, bytes_);
				engine::diag::CollectingSink sink {};
				const auto level = loadLevel(archive, name, sink);
				auto result = Loaded { .ok = level.has_value() && not sink.hasErrors(), .diagnostics = describe(sink), .resaved = {} };
				if (result.ok) {
					result.ok = level.value() != nullptr;
				}
				if (result.ok) {
					result.resaved = saveWith([&](mref<StructScopedSlot> slot_) { access::Structure<Level>::serialize(**level, ::hg::move(slot_)); });
				}
				return result;
			}
		};
	}

	/* Every type with a registered actor, component or asset scheme, plus the level root. */
	[[nodiscard]] Vector<Case> cases() {
		Vector<Case> out {};
		out.emplace_back(actorCase<Actor>("Actor", 0x01u));
		out.emplace_back(actorCase<StaticGeometryActor>("StaticGeometryActor", 0x03u));
		out.emplace_back(actorCase<BoxShapeGeometryActor>("BoxShapeGeometryActor", 0x04u));
		out.emplace_back(actorCase<SphereShapeGeometryActor>("SphereShapeGeometryActor", 0x05u));

		out.emplace_back(componentCase<HierarchyComponent>("HierarchyComponent"));
		out.emplace_back(componentCase<CameraComponent>("CameraComponent"));
		out.emplace_back(componentCase<StaticGeometryComponent>("StaticGeometryComponent"));
		out.emplace_back(componentCase<DirectionalLightComponent>("DirectionalLightComponent"));
		out.emplace_back(componentCase<PointLightComponent>("PointLightComponent"));
		out.emplace_back(componentCase<SpotLightComponent>("SpotLightComponent"));
		out.emplace_back(componentCase<SkyboxComponent>("SkyboxComponent"));
		// skipped: CameraActor, DirectionalLightActor, PointLightActor serialize crashes when built via ActorScheme::instantiate without a universe.

		out.emplace_back(assetCase<FontAsset>("FontAsset", [] { return make_uptr<FontAsset>(); }));
		out.emplace_back(assetCase<ImageAsset>("ImageAsset", [] { return make_uptr<ImageAsset>(); }));
		out.emplace_back(assetCase<TextureAsset>("TextureAsset", [] { return make_uptr<TextureAsset>(); }));
		out.emplace_back(assetCase<StaticGeometry>("StaticGeometry", [] { return make_uptr<StaticGeometry>(); }));
		out.emplace_back(
			assetCase<GraphicsMaterial>(
				"GraphicsMaterial",
				[] { return make_uptr<GraphicsMaterial>(AssetGuid {}, StringView { "material" }, AssetReferenceUrl {}, AssetUrl {}, AssetGuid {}); }
			)
		);
		out.emplace_back(
			assetCase<GraphicsMaterialPrototype>(
				"GraphicsMaterialPrototype",
				[] { return make_uptr<GraphicsMaterialPrototype>(AssetGuid {}, StringView { "prototype" }, AssetReferenceUrl {}, AssetUrl {}); }
			)
		);
		out.emplace_back(assetCase<LevelAsset>("LevelAsset", [] { return make_uptr<LevelAsset>(); }));

		out.emplace_back(levelCase("Level", false));
		out.emplace_back(levelCase("LevelWithActors", true));
		return out;
	}

	/* Constructed slot memory of archetype actors. */
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
			std::memset(_memory, 0, static_cast<size_t>(count_) * class_.desc().stride);
			_class.ops().construct(_memory, _count, stride(), ArchetypeInit { *Engine::getEngine()->getActors()->getRegistry() });
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

		[[nodiscard]] ptr<std::byte> slots() const noexcept {
			return _memory;
		}

	private:
		ref<const ActorClass> _class;
		u32 _count;
		ptr<std::byte> _memory;
	};

	constexpr u32 layoutCount = 2u;
	constexpr StringView layoutType = "TestTankLayout";

	class SchemaFixtures : public ::testing::Test {
	protected:
		uptr<test_engine::SpawnEngine> _engine;

		void SetUp() override {
			_engine = make_uptr<test_engine::SpawnEngine>();
			auto& module = *_engine->_serialization;
			activeSerial = _engine->_serialization.get();
			module.addActorScheme<Actor>();
			module.addActorScheme<CameraActor>();
			module.addActorScheme(StaticGeometryActor::typeId, MakeArchetypeFnSet(&StaticGeometryActor::actorClass()));
			module.addActorScheme(BoxShapeGeometryActor::typeId, MakeArchetypeFnSet(&BoxShapeGeometryActor::actorClass()));
			module.addActorScheme(SphereShapeGeometryActor::typeId, MakeArchetypeFnSet(&SphereShapeGeometryActor::actorClass()));
			module.addActorScheme<DirectionalLightActor>();
			module.addActorScheme<PointLightActor>();
			module.addComponentScheme<HierarchyComponent>();
			module.addComponentScheme<CameraComponent>();
			module.addComponentScheme<StaticGeometryComponent>();
			module.addComponentScheme<DirectionalLightComponent>();
			module.addComponentScheme<PointLightComponent>();
			module.addComponentScheme<SpotLightComponent>();
			module.addComponentScheme<SkyboxComponent>();
		}

		void TearDown() override {
			_engine.reset();
		}
	};

	TEST_F(SchemaFixtures, EveryTypeHasCurrentFixture) {
		const bool write = writeEnabled();
		for (const auto& entry : cases()) {
			const auto path = fixturePath(entry.name);
			if (fs::exists(path)) {
				continue;
			}
			if (write) {
				auto archive = BufferArchive {};
				entry.write(archive);
				writeFile(path, drain(archive));
				continue;
			}
			ADD_FAILURE() << "type '" << entry.name << "' has no revision 0 fixture at " << path.string() << "; run once with HG_WRITE_FIXTURES=1";
		}

		const auto layoutPath = fixturePath(layoutType);
		const auto hashPath = layoutPath.parent_path() / "layout-hash.txt";
		if (write && not fs::exists(layoutPath)) {
			const Slots source { TestTank::actorClass(), layoutCount };
			Bytes bytes {};
			TestTank::actorClass().ops().dumpLayout(source.slots(), layoutCount, source.stride(), bytes);
			writeFile(layoutPath, bytes);
		}
		if (write && not fs::exists(hashPath)) {
			const auto text = std::format("{:016x}\n", TestTank::actorClass().layoutHash());
			writeFile(hashPath, std::as_bytes(std::span { text.data(), text.size() }));
		}
		EXPECT_TRUE(fs::exists(layoutPath)) << layoutPath.string();
		EXPECT_TRUE(fs::exists(hashPath)) << hashPath.string();
	}

	TEST_F(SchemaFixtures, AllLoadWithoutError) {
		for (const auto& entry : cases()) {
			const auto bytes = readFile(fixturePath(entry.name));
			if (not bytes.has_value()) {
				ADD_FAILURE() << "missing fixture for '" << entry.name << "'";
				continue;
			}
			const auto loaded = entry.reload(*bytes);
			EXPECT_TRUE(loaded.ok) << "fixture '" << entry.name << "' failed to load:\n" << loaded.diagnostics;
		}
	}

	TEST_F(SchemaFixtures, CurrentSavesByteIdentical) {
		for (const auto& entry : cases()) {
			const auto bytes = readFile(fixturePath(entry.name));
			if (not bytes.has_value()) {
				ADD_FAILURE() << "missing fixture for '" << entry.name << "'";
				continue;
			}
			const auto loaded = entry.reload(*bytes);
			if (not loaded.ok) {
				ADD_FAILURE() << "fixture '" << entry.name << "' failed to load";
				continue;
			}
			EXPECT_EQ(loaded.resaved, *bytes) << "fixture '" << entry.name << "' does not save back byte identical";
		}
	}

	TEST_F(SchemaFixtures, ArchetypeLayoutHashPinned) {
		const auto layoutPath = fixturePath(layoutType);
		const auto text = readFile(layoutPath.parent_path() / "layout-hash.txt");
		ASSERT_TRUE(text.has_value());
		const auto pinned = std::strtoull(reinterpret_cast<const char*>(String(reinterpret_cast<const char*>(text->data()), text->size()).c_str()), nullptr, 16);
		ASSERT_EQ(pinned, TestTank::actorClass().layoutHash()) << "fields changed: re-cook layout data and update the hash";
	}

	TEST_F(SchemaFixtures, ArchetypeLayoutLoadsAndSavesByteIdentical) {
		const auto bytes = readFile(fixturePath(layoutType));
		ASSERT_TRUE(bytes.has_value());
		const Slots target { TestTank::actorClass(), layoutCount };
		ASSERT_TRUE(TestTank::actorClass().ops().loadLayout(target.slots(), layoutCount, target.stride(), *bytes).has_value())
			<< "fields changed: re-cook layout data and update the hash";
		Bytes again {};
		TestTank::actorClass().ops().dumpLayout(target.slots(), layoutCount, target.stride(), again);
		EXPECT_EQ(again, *bytes);
	}

	TEST(SchemaFixturesIds, TypeIdsPinned) {
		EXPECT_EQ(("Actor"_typeId).data, 15460027579171725742uLL);
		EXPECT_EQ(("CameraActor"_typeId).data, 10493670986011196821uLL);
		EXPECT_EQ(("Texture"_typeId).data, 7387711539637827580uLL);
		EXPECT_EQ(Actor::typeId.data, ("Actor"_typeId).data);
		EXPECT_EQ(CameraActor::typeId.data, ("CameraActor"_typeId).data);
		EXPECT_EQ(TextureAsset::typeId.data, ("Texture"_typeId).data);
	}

	/* Fuzz findings (seed 2605599740): byte 0 is the harness type selector, the rest the archive. */
	template <class ActorType_>
	[[nodiscard]] Loaded replayActorFinding(StringView file_, bool* returned_ = nullptr) {
		const auto bytes = readFile(fixtureDir() / "findings" / fs::path { file_ });
		if (not bytes.has_value() || bytes->empty()) {
			ADD_FAILURE() << "missing finding " << file_;
			return {};
		}
		const Bytes archive { bytes->begin() + 1, bytes->end() };
		return loadWith(
			archive,
			file_,
			[returned_](cref<RootScopedSlot> root_, ref<LoadContext>) {
				const auto scheme = serial().getActorScheme(ActorType_::typeId);
				const auto produced = scheme.has_value() && scheme->deserialize(root_, Engine::getEngine()->getActors()->getRegistry()) != nullptr;
				if (returned_ != nullptr) {
					*returned_ = produced;
				}
				return produced;
			}
		);
	}

	TEST_F(SchemaFixtures, FindingActorTypeMismatchRejected) {
		bool returned = true;
		const auto loaded = replayActorFinding<Actor>("ActorHydratePanic.bin", &returned);
		EXPECT_FALSE(returned) << "a rejected actor record must not yield an actor";
		EXPECT_FALSE(loaded.ok);
		EXPECT_TRUE(loaded.errors) << loaded.diagnostics;
		EXPECT_NE(loaded.diagnostics.find("[type-mismatch]"), String::npos) << loaded.diagnostics;
	}

	TEST_F(SchemaFixtures, FindingStaticGeometryActorRejectedWithError) {
		const auto loaded = replayActorFinding<StaticGeometryActor>("StaticGeometryActorSilent.bin");
		EXPECT_FALSE(loaded.ok);
		EXPECT_TRUE(loaded.errors) << loaded.diagnostics;
	}

	TEST_F(SchemaFixtures, UnresolvedAssetMarksArchetypeActorUnknownData) {
		auto source = sampleActor<StaticGeometryActor>(1u);
		auto geometry = static_cast<StaticGeometryActor&>(*source).getStaticGeometryComponent();
		ASSERT_TRUE(geometry != nullptr);
		struct Access : AssetHandle {
			static auto guidMember() noexcept { return &Access::_guid; }
		};
		StaticGeometryAssetHandle bare {};
		bare.*Access::guidMember() = AssetGuid { 1uL, 2u, 3u, 4uLL };
		geometry->setStaticGeometry(bare);

		auto archive = BufferArchive {}; {
			auto structured = StructuredArchive { archive };
			saveActor(*source, StaticGeometryActor::typeId, structured.insertRootSlot().intoStruct());
		}

		bool unknown = false;
		ptr<Actor> reloaded = nullptr;
		const auto loaded = loadWith(
			drain(archive),
			"unresolved",
			[&unknown, &reloaded](cref<RootScopedSlot> root_, ref<LoadContext>) {
				const auto scheme = serial().getActorScheme(StaticGeometryActor::typeId);
				auto* const actor = scheme->deserialize(root_, Engine::getEngine()->getActors()->getRegistry());
				unknown = actor != nullptr && actor->carriesUnknownData();
				reloaded = actor;
				return actor != nullptr;
			}
		);
		EXPECT_TRUE(loaded.ok) << loaded.diagnostics;
		EXPECT_TRUE(unknown);
		ASSERT_NE(reloaded, nullptr);

		// The mark has to reach the save boundary: a level holding the actor refuses to save.
		auto level = make_root_like_level();
		std::ignore = level->addActor(VolatileActor<> { ::hg::move(reloaded) });
		const auto saved = checkSaveAllowed(*level);
		ASSERT_FALSE(saved.has_value());
		ASSERT_EQ(saved.error().size(), 1uLL);
		EXPECT_EQ(String { saved.error().front().code()->key }, "save-refused");
		ASSERT_EQ(saved.error().front().notes().size(), 1uLL);
		EXPECT_NE(saved.error().front().notes().front().find("actor"), String::npos);

		// A clean level next to it still saves.
		EXPECT_TRUE(checkSaveAllowed(*make_root_like_level()).has_value());
	}
}
