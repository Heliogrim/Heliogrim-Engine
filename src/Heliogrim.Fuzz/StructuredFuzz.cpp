/**
 * Feeds arbitrary bytes through the load boundary of real types: open (root slot), validate (migration, context) and hydrate.
 * The first input byte selects the type, the rest is the archive. Every input must end accepted, or rejected with diagnostics.
 */

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <functional>
#include <iterator>
#include <string>
#include <utility>
#include <Engine.Assets.Type/AssetLoad.hpp>
#include <Engine.Assets.Type/Geometry/StaticGeometry.hpp>
#include <Engine.Assets.Type/Texture/FontAsset.hpp>
#include <Engine.Assets.Type/Texture/ImageAsset.hpp>
#include <Engine.Assets.Type/Texture/TextureAsset.hpp>
#include <Engine.Assets.Type/Universe/LevelAsset.hpp>
#include <Engine.Diagnostics/Sink.hpp>
#include <Engine.Level/Level.hpp>
#include <Engine.Resource.Archive/BufferArchive.hpp>
#include <Engine.Serialization/Access/Structure.hpp>
#include <Engine.Serialization/Load/LoadContext.hpp>
#include <Engine.Serialization/Scheme/ActorScheme.hpp>
#include <Engine.Serialization/Scheme/ComponentScheme.hpp>
#include <Engine.Serialization/Structure/RootScopedSlot.hpp>
#include <Heliogrim/Actor/Actor.hpp>
#include <Heliogrim/Actor/Archetype/ArchetypeFnSet.hpp>
#include <Heliogrim/Actor/Geometry/BoxShapeGeometryActor.hpp>
#include <Heliogrim/Actor/Geometry/SphereShapeGeometryActor.hpp>
#include <Heliogrim/Actor/Geometry/StaticGeometryActor.hpp>
#include <Heliogrim/Component/HierarchyComponent.hpp>
#include <Heliogrim/Component/Scene/Camera/CameraComponent.hpp>
#include <Heliogrim/Component/Scene/Geometry/StaticGeometryComponent.hpp>
#include <Heliogrim/Component/Scene/Light/DirectionalLightComponent.hpp>
#include <Heliogrim/Component/Scene/Light/PointLightComponent.hpp>
#include <Heliogrim/Component/Scene/Light/SpotLightComponent.hpp>
#include <Heliogrim.Test/SpawnEngine.hpp>

using namespace hg;
using namespace hg::engine::serialization;
using namespace hg::engine::resource;
using namespace hg::engine::assets;
using namespace hg::engine::core;

namespace {
	using Hydrate = std::function<bool(ref<const RootScopedSlot>, ref<LoadContext>)>;

	struct Type {
		ptr<const char> name;
		Hydrate hydrate;
	};

	ref<HeliogrimApi::test_engine::SpawnEngine> spawned() {
		// shortcut: never destroyed, pooled actors leak (known default-deleter crash); fine for a fuzz process.
		static auto* const instance = [] {
			auto* const self = new HeliogrimApi::test_engine::SpawnEngine {};
			auto& module = *self->_serialization;
			module.addActorScheme<Actor>();
			module.addActorScheme(StaticGeometryActor::typeId, MakeArchetypeFnSet(&StaticGeometryActor::actorClass()));
			module.addActorScheme(BoxShapeGeometryActor::typeId, MakeArchetypeFnSet(&BoxShapeGeometryActor::actorClass()));
			module.addActorScheme(SphereShapeGeometryActor::typeId, MakeArchetypeFnSet(&SphereShapeGeometryActor::actorClass()));
			module.addComponentScheme<HierarchyComponent>();
			module.addComponentScheme<CameraComponent>();
			module.addComponentScheme<StaticGeometryComponent>();
			module.addComponentScheme<DirectionalLightComponent>();
			module.addComponentScheme<PointLightComponent>();
			module.addComponentScheme<SpotLightComponent>();
			return self;
		}();
		return *instance;
	}

	template <class ActorType_>
	Type actorType(ptr<const char> name_) {
		return Type {
			name_,
			[](ref<const RootScopedSlot> root_, ref<LoadContext> ctx_) {
				const auto scheme = spawned()._serialization->getActorScheme(ActorType_::typeId);
				// shortcut: the pooled actor is never destroyed (known default-deleter crash), leak until fixed.
				const ptr<Actor> actor = scheme->deserialize(root_, spawned().getActors()->getRegistry());
				return actor != nullptr && not ctx_.failed();
			}
		};
	}

	template <class ComponentType_>
	Type componentType(ptr<const char> name_) {
		return Type {
			name_,
			[](ref<const RootScopedSlot> root_, ref<LoadContext> ctx_) {
				const auto target = make_uptr<ComponentType_>(ComponentGuid {}, CachedActorPointer {}, nullptr);
				access::Structure<ComponentType_>::hydrate(root_.asStruct(), *target);
				return not ctx_.failed();
			}
		};
	}

	template <class AssetType_>
	Type assetType(ptr<const char> name_) {
		return Type {
			name_,
			[](ref<const RootScopedSlot> root_, ref<LoadContext> ctx_) {
				AssetType_ target {};
				return hydrateArchivedAsset(spawned()._serialization->migrations(), root_.asStruct(), ctx_, target);
			}
		};
	}

	/* Fixture names match src/Heliogrim.Test/fixtures/serialization/<name>/r0.bin. Level goes through loadLevel (index 0). */
	const Vector<Type>& types() {
		static const Vector<Type> table {
			Type { "LevelWithActors", {} },
			actorType<Actor>("Actor"),
			actorType<StaticGeometryActor>("StaticGeometryActor"),
			actorType<BoxShapeGeometryActor>("BoxShapeGeometryActor"),
			actorType<SphereShapeGeometryActor>("SphereShapeGeometryActor"),
			componentType<HierarchyComponent>("HierarchyComponent"),
			componentType<CameraComponent>("CameraComponent"),
			componentType<StaticGeometryComponent>("StaticGeometryComponent"),
			componentType<DirectionalLightComponent>("DirectionalLightComponent"),
			componentType<PointLightComponent>("PointLightComponent"),
			componentType<SpotLightComponent>("SpotLightComponent"),
			assetType<FontAsset>("FontAsset"),
			assetType<ImageAsset>("ImageAsset"),
			assetType<TextureAsset>("TextureAsset"),
			assetType<StaticGeometry>("StaticGeometry"),
			assetType<LevelAsset>("LevelAsset")
		};
		return table;
	}
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* const data_, const size_t size_) {
	if (size_ == 0uLL) {
		return 0;
	}
	(void)spawned();
	const auto& table = types();
	const auto& type = table[data_[0] % table.size()];

	auto archive = BufferArchive {};
	auto bytes = Vector<std::byte>(reinterpret_cast<const std::byte*>(data_ + 1), reinterpret_cast<const std::byte*>(data_ + size_));
	archive.serializeBytes(bytes.data(), bytes.size(), ArchiveStreamMode::eStore);
	archive.seek(0);
	engine::diag::CollectingSink sink {};

	bool accepted = false;
	if (not type.hydrate) {
		const auto level = loadLevel(archive, type.name, sink);
		accepted = level.has_value() && level.value() != nullptr;
	} else {
		auto ctx = LoadContext { static_cast<u64>(archive.totalSize()), String { type.name } };
		bool hydrated = false;
		{
			const auto root = RootScopedSlot { archive, ScopedSlotStateFlag::eClean, &ctx };
			hydrated = type.hydrate(root, ctx);
		}
		auto finished = ctx.finish(sink, "while fuzzing");
		if (not finished.has_value()) {
			for (auto& diagnostic : finished.error()) {
				sink.emit(::hg::move(diagnostic));
			}
		}
		accepted = finished.has_value() && hydrated;
	}

	// Invariant: accepted carries no errors, rejected carries at least one error-severity diagnostic.
	if ((accepted && sink.hasErrors()) || (not accepted && not sink.hasErrors())) {
		std::fprintf(stderr, "invariant broken: type=%s accepted=%d errors=%d diagnostics=%zu\n", type.name, accepted, sink.hasErrors(), sink.items().size());
		std::abort();
	}
	return 0;
}

/* Seed: the fixture of HG_FUZZ_SEED_TYPE (default LevelWithActors), prefixed with its type index. */
extern "C" size_t HgFuzzSeed(uint8_t* const out_, const size_t cap_) {
	const auto& table = types();
	ptr<const char> const wanted = std::getenv("HG_FUZZ_SEED_TYPE");
	size_t index = 0uLL;
	for (size_t i = 0uLL; wanted != nullptr && i < table.size(); ++i) {
		if (std::strcmp(table[i].name, wanted) == 0) {
			index = i;
		}
	}
	std::ifstream in { std::string { HG_FIXTURE_DIR } + "/" + table[index].name + "/r0.bin", std::ios::binary };
	const std::string raw { std::istreambuf_iterator<char> { in }, std::istreambuf_iterator<char> {} };
	if (cap_ == 0uLL) {
		return 0uLL;
	}
	out_[0] = static_cast<uint8_t>(index);
	const size_t n = raw.size() + 1uLL < cap_ ? raw.size() : cap_ - 1uLL;
	std::memcpy(out_ + 1, raw.data(), n);
	return n + 1uLL;
}
