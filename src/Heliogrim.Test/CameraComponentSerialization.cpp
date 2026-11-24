#include "pch.h"

/**/

#include <Engine.ACS/Archetype/ArchetypeLayout.inl.hpp>
import Engine.Common;
import Engine.Graphics.Components;
import Engine.Resource.Archive;
import Engine.Serialization;
import Engine.Serialization.Structures;

using namespace hg::engine::serialization;
using namespace hg::engine::resource;
using namespace hg;

namespace {
	template <typename Type_>
	[[nodiscard]] uptr<Type_> make() {
		return make_uptr<Type_>(ComponentGuid {}, CachedActorPointer {}, nullptr);
	}

	template <typename Type_>
	void structuredRoundTrip(ref<const Type_> source_, ref<Type_> target_) {
		auto archive = BufferArchive {}; {
			auto structured = StructuredArchive { archive };
			access::Structure<Type_>::serialize(source_, structured.insertRootSlot().intoStruct());
		}
		archive.seek(0);
		auto structured = StructuredArchive { archive };
		access::Structure<Type_>::hydrate(structured.getRootSlot().asStruct(), target_);
	}

	template <typename Type_>
	void layoutRoundTrip(ref<const Type_> source_, ref<Type_> target_) {
		Vector<std::byte> bytes {};
		::hg::engine::acs::dump_payload(source_, bytes);
		ASSERT_EQ(bytes.size(), ::hg::engine::acs::payload_size<Type_>());
		ASSERT_EQ(::hg::engine::acs::check_payload<Type_>(bytes, 0uLL), ~0uLL);
		::hg::engine::acs::commit_payload(bytes, 0uLL, target_);
		LoadContext ctx { bytes.size() };
		::hg::engine::acs::resolve_payload(bytes, 0uLL, target_, ctx);
	}

	[[nodiscard]] uptr<CameraComponent> makeWithModes() {
		auto camera = make<CameraComponent>();
		camera->setProjectMode(CameraComponent::CameraProjectMode::eOrthographic);
		camera->setViewMode(CameraComponent::CameraViewMode::eLookAt);
		return camera;
	}
}

TEST(CameraComponentSerialization, StructuredRoundTripKeepsModes) {
	const auto source = makeWithModes();
	const auto target = make<CameraComponent>();
	structuredRoundTrip(*source, *target);
	EXPECT_EQ(target->getProjectMode(), CameraComponent::CameraProjectMode::eOrthographic);
	EXPECT_EQ(target->getViewMode(), CameraComponent::CameraViewMode::eLookAt);
}

TEST(CameraComponentSerialization, LayoutRoundTripKeepsModes) {
	const auto source = makeWithModes();
	const auto target = make<CameraComponent>();
	layoutRoundTrip(*source, *target);
	EXPECT_EQ(target->getProjectMode(), CameraComponent::CameraProjectMode::eOrthographic);
	EXPECT_EQ(target->getViewMode(), CameraComponent::CameraViewMode::eLookAt);
}

namespace {
	// Mirrors Structure<CameraComponent>::serialize, but stores the modes as the given raw s32 values.
	struct Hydrated {
			Opt<ReadError> error;
			uptr<CameraComponent> target;
		};

		[[nodiscard]] Hydrated hydrateWithModes(const s32 projectMode_, const s32 viewMode_) {
		auto archive = BufferArchive {}; {
			const auto source = make<CameraComponent>();
			auto structured = StructuredArchive { archive };
			auto record = structured.insertRootSlot().intoStruct();
			record.insertSlot<f32>("fov") << 90.F;
			record.insertSlot<f32>("aspectRation") << 1.F;
			record.insertSlot<bool>("lockedAspect") << false;
			record.insertSlot<bool>("visibleFrustum") << false;
			access::Structure<math::fvec3>::serialize(math::fvec3 {}, record.insertStructSlot("offset"));
			record.insertSlot<s32>("projectMode") << projectMode_;
			record.insertSlot<s32>("viewMode") << viewMode_;
			access::Structure<HierarchyComponent>::serialize(*source, ::hg::move(record));
		}
		archive.seek(0);

		auto ctx = LoadContext { static_cast<u64>(archive.totalSize()) };
		auto target = make<CameraComponent>(); {
			const auto root = RootScopedSlot { archive, ScopedSlotStateFlag::eClean, &ctx };
			access::Structure<CameraComponent>::hydrate(root.asStruct(), *target);
		}
		return Hydrated {
				ctx.firstError() != nullptr ? Opt<ReadError> { *ctx.firstError() } : Opt<ReadError> {},
				::hg::move(target)
			};
	}
}

TEST(CameraComponentSerialization, ModesInRangeLoad) {
	const auto hydrated = hydrateWithModes(1, 1);
	EXPECT_FALSE(hydrated.error.has_value());
	EXPECT_EQ(hydrated.target->getProjectMode(), CameraComponent::CameraProjectMode::eOrthographic);
	EXPECT_EQ(hydrated.target->getViewMode(), CameraComponent::CameraViewMode::eLookAt);
}

TEST(CameraComponentSerialization, ProjectModeOutOfRangeRejected) {
	const auto error = hydrateWithModes(2, 0).error;
	ASSERT_TRUE(error.has_value());
	EXPECT_EQ(error->kind, ReadError::Kind::eInvalidValue);
	EXPECT_EQ(error->detail, "camera projection mode is out of range");
}

TEST(CameraComponentSerialization, ViewModeOutOfRangeRejected) {
	const auto error = hydrateWithModes(0, -1).error;
	ASSERT_TRUE(error.has_value());
	EXPECT_EQ(error->kind, ReadError::Kind::eInvalidValue);
	EXPECT_EQ(error->detail, "camera view mode is out of range");
}
