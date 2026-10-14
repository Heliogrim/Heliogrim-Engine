#include "pch.h"
/**/

#include <vector>
#include <Engine.Reflect/Inherit/InheritBase.hpp>
#include <Engine.Reflect/Inherit/InheritMeta.hpp>
#include <Engine.Scene.Support/SceneSystemTypeMappedRegistry.hpp>
#include <Heliogrim/Component/SceneComponent.hpp>

using namespace hg::engine::scene;
using namespace hg;

namespace SceneSupportModule {
	class ModelBase :
		public InheritBase<ModelBase> {
	public:
		inline constexpr static hg::type_id typeId { "SceneSupportModelBase"_typeId };
	};

	class ModelA final :
		public InheritMeta<ModelA, ModelBase> {
	public:
		inline constexpr static hg::type_id typeId { "SceneSupportModelA"_typeId };

		explicit ModelA(ref<SceneComponent> owner_) noexcept :
			owner(&owner_) {}

		ptr<SceneComponent> owner;
	};

	class ModelB final :
		public InheritMeta<ModelB, ModelBase> {
	public:
		inline constexpr static hg::type_id typeId { "SceneSupportModelB"_typeId };

		explicit ModelB(ref<SceneComponent> owner_) noexcept :
			owner(&owner_) {}

		ptr<SceneComponent> owner;
	};

	struct Padding {
		virtual ~Padding() = default;

		char pad[16] {};
	};

	// Leading polymorphic base pushes the `ModelBase` subobject to a non-zero offset.
	class ModelOffset final :
		public Padding,
		public InheritMeta<ModelOffset, ModelBase> {
	public:
		inline constexpr static hg::type_id typeId { "SceneSupportModelOffset"_typeId };

		explicit ModelOffset(ref<SceneComponent> owner_) noexcept :
			owner(&owner_) {}

		ptr<SceneComponent> owner;
	};

	class CompA final :
		public InheritMeta<CompA, SceneComponent> {
	public:
		inline constexpr static hg::type_id typeId { "SceneSupportCompA"_typeId };

		CompA() :
			InheritMeta(ComponentGuid {}, ComponentTypeId {}, CachedActorPointer {}, nullptr) {}
	};

	class CompB final :
		public InheritMeta<CompB, SceneComponent> {
	public:
		inline constexpr static hg::type_id typeId { "SceneSupportCompB"_typeId };

		CompB() :
			InheritMeta(ComponentGuid {}, ComponentTypeId {}, CachedActorPointer {}, nullptr) {}
	};

	using Registry = SceneSystemTypeMappedRegistry<ModelBase>;
	using Storage = SceneSystemTypeMappedStorage<ModelA, ModelBase>;

	/**/

	TEST(SceneSystemTypeMappedStorage, AddAndForEach) {
		Storage storage {};
		CompA comp;

		EXPECT_EQ(&storage.getDataMetaClass(), ModelA::getStaticMetaClass());

		const auto first = storage.add(comp);
		const auto second = storage.add(comp);
		EXPECT_NE(first.get(), nullptr);
		EXPECT_NE(first.get(), second.get());

		size_t count = 0;
		storage.forEach([&](ref<const ModelBase>) { ++count; });
		EXPECT_EQ(count, 2uLL);
	}

	TEST(SceneSystemTypeMappedStorage, RemoveByPointer) {
		Storage storage {};
		CompA a, b, c;

		storage.add(a);
		storage.add(b);
		storage.add(c);

		// `add` results are invalidated by reallocation, so resolve the element only after all adds.
		ptr<const ModelBase> mid = nullptr;
		storage.forEach([&](ref<const ModelA> m_) { mid = m_.owner == &b ? &m_ : mid; });
		ASSERT_NE(mid, nullptr);

		storage.remove(::hg::move(mid));

		std::vector<ptr<SceneComponent>> owners {};
		storage.forEach(
			[&](ref<const ModelBase> m_) { owners.push_back(static_cast<ref<const ModelA>>(m_).owner); }
		);
		ASSERT_EQ(owners.size(), 2uLL);
		EXPECT_EQ(owners[0], &a);
		EXPECT_EQ(owners[1], &c);
	}

	TEST(SceneSystemTypeMappedStorage, RemoveByPointerNonZeroBaseOffset) {
		SceneSystemTypeMappedStorage<ModelOffset, ModelBase> storage {};
		CompA a, b, c;

		storage.add(a);
		storage.add(b);
		storage.add(c);

		ptr<const ModelBase> mid = nullptr;
		storage.forEach(
			[&](ref<const ModelOffset> m_) {
				if (m_.owner == &b) {
					// Guard: the base subobject must not share the model's address.
					EXPECT_NE(static_cast<const void*>(&m_), static_cast<const void*>(static_cast<const ModelBase*>(&m_)));
					mid = &m_;
				}
			}
		);
		ASSERT_NE(mid, nullptr);

		storage.remove(::hg::move(mid));

		std::vector<ptr<SceneComponent>> owners {};
		storage.forEach([&](ref<const ModelOffset> m_) { owners.push_back(m_.owner); });
		ASSERT_EQ(owners.size(), 2uLL);
		EXPECT_EQ(owners[0], &a);
		EXPECT_EQ(owners[1], &c);
	}

	TEST(SceneSystemTypeMappedStorage, RemoveBySelector) {
		Storage storage {};
		CompA a, b, c;

		storage.add(a);
		storage.add(b);
		storage.add(c);

		storage.remove(
			FnRef<bool(ref<const ModelBase>)> {
				[&](ref<const ModelBase> m_) { return static_cast<ref<const ModelA>>(m_).owner == &b; }
			}
		);

		size_t count = 0;
		storage.forEachMut(
			[&](ref<ModelA> m_) {
				++count;
				EXPECT_NE(m_.owner, &b);
			}
		);
		EXPECT_EQ(count, 2uLL);
	}

	/**/

	TEST(SceneSystemTypeMappedRegistry, MappingsAndStorageLookup) {
		Registry registry {};
		registry.addTypeMapping<CompA, ModelA>();
		registry.addTypeMapping<CompB, ModelB>();

		EXPECT_EQ(registry.getTypeMappings().size(), 2uLL);
		EXPECT_EQ(std::ranges::distance(registry.getTypeStorages()), 2);

		const auto found = registry.findTypeStorage(ModelA::getStaticMetaClass());
		ASSERT_TRUE(found.has_value());
		EXPECT_EQ(&found.value().getDataMetaClass(),ModelA::getStaticMetaClass());

		EXPECT_FALSE(registry.findTypeStorage(CompA::getStaticMetaClass()).has_value());
	}

	TEST(SceneSystemTypeMappedRegistry, SharedDestinationSingleStorage) {
		Registry registry {};
		registry.addTypeMapping<CompA, ModelA>();
		registry.addTypeMapping<CompB, ModelA>();

		EXPECT_EQ(registry.getTypeMappings().size(), 2uLL);
		EXPECT_EQ(std::ranges::distance(registry.getTypeStorages()), 1);
	}

	TEST(SceneSystemTypeMappedRegistry, ForEachTyped) {
		Registry registry {};
		registry.addTypeMapping<CompA, ModelA>();

		CompA comp;
		registry.findTypeStorage(ModelA::getStaticMetaClass()).value().add(comp);

		size_t count = 0;
		const auto& cregistry = registry;
		cregistry.forEach<ModelA>([&](ref<const ModelA>) { ++count; });
		EXPECT_EQ(count, 1uLL);
	}
}
