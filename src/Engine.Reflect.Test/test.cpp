#include "pch.h"

#include <Engine.Common/Meta/Constexpr.hpp>
#include <Engine.Reflect/PartialTypeInfo.hpp>
#include <Engine.Reflect/TypeId.hpp>
#include <Engine.Reflect/TypeName.hpp>
#include <Engine.Reflect/Compile/HashMap.hpp>
#include <Engine.Reflect/Compile/Map.hpp>
#include <Engine.Reflect/Compile/TypeId.hpp>
#include <Engine.Reflect/Compile/TypeName.hpp>
#include <Engine.Reflect/Inherit/InheritBase.hpp>
#include <Engine.Reflect/Inherit/InheritMeta.hpp>
#include <Engine.Reflect/Meta/TypedMetaClass.hpp>

using namespace hg;

namespace ReflectModule {
	class TestBase01 : public InheritBase<TestBase01> {
	public:
		inline constexpr static hg::type_id typeId { "TestBase01"_typeId };
	};

	class TestDerived01 : public InheritMeta<TestDerived01, TestBase01> {
	public:
		inline constexpr static hg::type_id typeId { "TestDerived01"_typeId };
	};

	class TestDerived01F final : public InheritMeta<TestDerived01F, TestBase01> {
	public:
		inline constexpr static hg::type_id typeId { "TestDerived01F"_typeId };
	};

	class TestDerived02 : public InheritMeta<TestDerived02, TestDerived01> {
	public:
		inline constexpr static hg::type_id typeId { "TestDerived02"_typeId };
	};

	class TestDerived02F final : public InheritMeta<TestDerived02F, TestDerived01> {
	public:
		inline constexpr static hg::type_id typeId { "TestDerived02F"_typeId };
	};

	TEST(HeliogrimObject, Create) {
		//
		//auto obj { HeliogrimObject::create<TestBase01>() };
		auto obj = new TestBase01();

		//
		EXPECT_NE(obj->getMetaClass(), nullptr);

		//
		delete obj;
	}

	TEST(HeliogrimObject, FinalIdentity) {
		//
		//auto obj { HeliogrimObject::create<TestDerived01F>() };
		auto obj = new TestDerived01F();

		// Same Class Check
		EXPECT_TRUE(obj->getMetaClass()->is<TestDerived01F>());
		EXPECT_TRUE(obj->getMetaClass()->exact<TestDerived01F>());
		EXPECT_EQ(::hg::refl::TypeId<TestDerived01F>(), obj->getMetaClass()->typeId());

		// Outer Class Check
		EXPECT_FALSE(obj->getMetaClass()->is<TestDerived02F>());
		EXPECT_FALSE(obj->getMetaClass()->exact<TestDerived02F>());
		EXPECT_NE(::hg::refl::TypeId<TestDerived02F>(), obj->getMetaClass()->typeId());

		// Base Class Check
		//EXPECT_TRUE(obj->getMetaClass()->isType<TestBase01>());
		EXPECT_FALSE(obj->getMetaClass()->exact<TestBase01>());
		EXPECT_NE(::hg::refl::TypeId<TestBase01>(), obj->getMetaClass()->typeId());

		//
		delete obj;
	}
}

namespace ReflectModule {
	class TestMetaBase : public InheritBase<TestMetaBase> {
	public:
		constexpr TestMetaBase() :
			InheritBase<TestMetaBase>() {}
	};

	class TestMetaDerived : public InheritMeta<TestMetaDerived, TestMetaBase> {
	public:
		constexpr TestMetaDerived() = default;
	};

	class TestExtBase {
	public:
		constexpr TestExtBase() = default;
	};

	class TestMetaPolyDerived : public InheritMeta<TestMetaPolyDerived, TestMetaBase, TestExtBase> {
	public:
		constexpr TestMetaPolyDerived() = default;
	};

	class TestMetaBase2 : public InheritBase<TestMetaBase2> {
	public:
		constexpr TestMetaBase2() :
			InheritBase<TestMetaBase2>() {}
	};

	class TestPolyMetaDerived : public InheritMeta<TestPolyMetaDerived, TestMetaBase, TestMetaBase2> {
	public:
		TestPolyMetaDerived() = default;
	};

	class TestPolyMetaPolyDerived : public InheritMeta<TestPolyMetaPolyDerived, TestMetaPolyDerived, TestMetaBase2> {
	public:
		TestPolyMetaPolyDerived() = default;
	};

	TEST(MetaInherit, BaseInheritMechanism) {
		//
		auto obj0 = TestMetaBase();
		auto obj1 = TestMetaDerived();

		//
		EXPECT_NE(obj0.getMetaClass(), nullptr);
		EXPECT_NE(obj1.getMetaClass(), nullptr);

		EXPECT_NE(obj0.getMetaClass()->typeId(), obj1.getMetaClass()->typeId());

		//
		std::cout << "obj0->meta->name :: " << obj0.getMetaClass()->typeId().data << std::endl;
		std::cout << "obj1->meta->name :: " << obj1.getMetaClass()->typeId().data << std::endl;
	}

	TEST(MetaInherit, AdvancedInheritSM) {

		/**/
		auto obj0 = TestMetaPolyDerived();
		auto obj1 = TestMetaBase();
		[[maybe_unused]] auto obj2 = TestExtBase();

		auto obj3 = TestMetaDerived();

		/**/

		EXPECT_NE(obj0.getMetaClass()->typeId(), obj1.getMetaClass()->typeId());
		EXPECT_TRUE(obj0.getMetaClass()->inherits(TestMetaBase::meta_class<>::get()));

		EXPECT_TRUE(obj0.getMetaClass()->inherits(::hg::refl::ctid<TestExtBase>()));
	}

	TEST(MetaInherit, TestPolyMetaDerived) {

		/**/
		auto obj0 = TestPolyMetaDerived();
		auto obj1 = TestMetaBase();
		auto obj2 = TestMetaBase2();

		auto obj3 = TestMetaDerived();

		/**/

		EXPECT_NE(obj0.getMetaClass()->typeId(), obj1.getMetaClass()->typeId());
		EXPECT_TRUE(obj0.getMetaClass()->inherits(TestMetaBase::meta_class<>::get()));

		EXPECT_NE(obj0.getMetaClass()->typeId(), obj2.getMetaClass()->typeId());
		EXPECT_TRUE(obj0.getMetaClass()->inherits(TestMetaBase2::meta_class<>::get()));
	}

	TEST(MetaInherit, DeepInherit) {

		/**/
		auto obj0 = TestPolyMetaPolyDerived();
		auto obj1 = TestMetaPolyDerived();

		auto obj2 = TestMetaBase();
		[[maybe_unused]] auto obj3 = TestExtBase();

		/**/

		EXPECT_NE(obj0.getMetaClass()->typeId(), obj1.getMetaClass()->typeId());
		EXPECT_TRUE(obj0.getMetaClass()->inherits(TestMetaPolyDerived::meta_class<>::get()));

		EXPECT_NE(obj0.getMetaClass()->typeId(), obj2.getMetaClass()->typeId());
		EXPECT_TRUE(obj0.getMetaClass()->inherits(TestMetaBase::meta_class<>::get()));

		EXPECT_TRUE(obj0.getMetaClass()->inherits(::hg::refl::ctid<TestExtBase>()));
	}

	TEST(MetaInherit, StaticMeta) {

		// TODO: Currently we are bound to already instantiated objects, cause otherwise,
		// TODO:    we can not aggregate the required information.
		// TODO:    In the future, we want to achieve this at compile time.

		// Warning: The following instruction will break the whole reflected structure currently:
		EXPECT_TRUE(
			TestPolyMetaPolyDerived::meta_class<>::get()->inherits(TestMetaBase::meta_class<>::get())
		);
	}
}

/**/

namespace ReflectModule {
	TEST(CompileTime, AllNameEqual) {

		struct TestType {
			int data[4];
		};

		constexpr auto deep = ::hg::refl::ctname<TestType>();
		constexpr auto forwarded = ::hg::refl::TypeName<TestType>();

		EXPECT_EQ(deep, forwarded);
	}

	TEST(CompileTime, AllTypeIdEqual) {

		struct TestType {
			float data[4];
		};

		constexpr auto deep = ::hg::refl::ctid<TestType>();
		constexpr auto* metaClass = ::hg::TypedMetaClass<TestType>::get();
		constexpr auto partialInfo = ::hg::refl::PartialTypeInfo::from<TestType>();
		constexpr auto fullInfo = ::hg::refl::FullTypeInfo::from<TestType>();
		constexpr auto queried = ::hg::refl::TypeId<TestType>();

		EXPECT_EQ(deep, metaClass->typeId());
		EXPECT_EQ(deep, partialInfo.meta->typeId());
		EXPECT_EQ(deep, fullInfo.meta->typeId());
		EXPECT_EQ(deep, queried);
	}
}

/**/

struct mapped_load {
	u64 marker;
};

static consteval auto test_gen_map() {
	return make_compile_map<string_view, mapped_load>(
		std::pair<string_view, mapped_load> { "b"sv, {} },
		std::pair<string_view, mapped_load> { "c"sv, {} },
		std::pair<string_view, mapped_load> { "a"sv, {} },
		std::pair<string_view, mapped_load> { "d"sv, {} },
		std::pair<string_view, mapped_load> { "f"sv, {} },
		std::pair<string_view, mapped_load> { "e"sv, {} },
		//
		std::pair<string_view, mapped_load> { "y"sv, {} },
		std::pair<string_view, mapped_load> { "y"sv, {} }
	);
}

static consteval auto test_gen_hash_map() {
	return make_compile_hash_map<string_view, mapped_load>(
		std::pair<string_view, mapped_load> { "b"sv, {} },
		std::pair<string_view, mapped_load> { "c"sv, {} },
		std::pair<string_view, mapped_load> { "a"sv, {} },
		std::pair<string_view, mapped_load> { "d"sv, {} },
		std::pair<string_view, mapped_load> { "f"sv, {} },
		std::pair<string_view, mapped_load> { "e"sv, {} },
		//
		std::pair<string_view, mapped_load> { "y"sv, {} },
		std::pair<string_view, mapped_load> { "y"sv, {} }
	);
}

static consteval auto test_gen_mass_hash_map() {
	return make_compile_hash_map<string_view, mapped_load>(
		std::pair<string_view, mapped_load> { "first-entry"sv, {} },
		std::pair<string_view, mapped_load> { "mass-entry"sv, {} },
		std::pair<string_view, mapped_load> { "mass-entry"sv, {} },
		std::pair<string_view, mapped_load> { "mass-entry"sv, {} },
		std::pair<string_view, mapped_load> { "mass-entry"sv, {} },
		std::pair<string_view, mapped_load> { "mass-entry"sv, {} },
		std::pair<string_view, mapped_load> { "mass-entry"sv, {} },
		std::pair<string_view, mapped_load> { "mass-entry"sv, {} },
		std::pair<string_view, mapped_load> { "mass-entry"sv, {} },
		std::pair<string_view, mapped_load> { "mass-entry"sv, {} },
		std::pair<string_view, mapped_load> { "mass-entry"sv, {} },
		std::pair<string_view, mapped_load> { "mass-entry"sv, {} },
		std::pair<string_view, mapped_load> { "mass-entry"sv, {} },
		std::pair<string_view, mapped_load> { "mass-entry"sv, {} },
		std::pair<string_view, mapped_load> { "mass-entry"sv, {} },
		std::pair<string_view, mapped_load> { "mass-entry"sv, {} },
		std::pair<string_view, mapped_load> { "mass-entry"sv, {} },
		std::pair<string_view, mapped_load> { "mass-entry"sv, {} },
		std::pair<string_view, mapped_load> { "mass-entry"sv, {} },
		std::pair<string_view, mapped_load> { "mass-entry"sv, {} },
		std::pair<string_view, mapped_load> { "mass-entry"sv, {} },
		std::pair<string_view, mapped_load> { "mass-entry"sv, {} },
		std::pair<string_view, mapped_load> { "mass-entry"sv, {} },
		std::pair<string_view, mapped_load> { "mass-entry"sv, {} },
		std::pair<string_view, mapped_load> { "mass-entry"sv, {} },
		std::pair<string_view, mapped_load> { "mass-entry"sv, {} },
		std::pair<string_view, mapped_load> { "mass-entry"sv, {} },
		std::pair<string_view, mapped_load> { "mass-entry"sv, {} },
		std::pair<string_view, mapped_load> { "mass-entry"sv, {} },
		std::pair<string_view, mapped_load> { "mass-entry"sv, {} },
		std::pair<string_view, mapped_load> { "mass-entry"sv, {} },
		std::pair<string_view, mapped_load> { "mass-entry"sv, {} },
		std::pair<string_view, mapped_load> { "mass-entry"sv, {} },
		std::pair<string_view, mapped_load> { "mass-entry"sv, {} },
		std::pair<string_view, mapped_load> { "mass-entry"sv, {} },
		std::pair<string_view, mapped_load> { "mass-entry"sv, {} },
		std::pair<string_view, mapped_load> { "mass-entry"sv, {} },
		std::pair<string_view, mapped_load> { "mass-entry"sv, {} },
		std::pair<string_view, mapped_load> { "mass-entry"sv, {} },
		std::pair<string_view, mapped_load> { "mass-entry"sv, {} },
		std::pair<string_view, mapped_load> { "mass-entry"sv, {} },
		std::pair<string_view, mapped_load> { "mass-entry"sv, {} },
		std::pair<string_view, mapped_load> { "mass-entry"sv, {} },
		std::pair<string_view, mapped_load> { "mass-entry"sv, {} },
		std::pair<string_view, mapped_load> { "mass-entry"sv, {} },
		std::pair<string_view, mapped_load> { "mass-entry"sv, {} },
		std::pair<string_view, mapped_load> { "mass-entry"sv, {} },
		std::pair<string_view, mapped_load> { "mass-entry"sv, {} },
		std::pair<string_view, mapped_load> { "mass-entry"sv, {} },
		std::pair<string_view, mapped_load> { "mass-entry"sv, {} },
		std::pair<string_view, mapped_load> { "mass-entry"sv, {} },
		std::pair<string_view, mapped_load> { "mass-entry"sv, {} },
		std::pair<string_view, mapped_load> { "mass-entry"sv, {} },
		std::pair<string_view, mapped_load> { "mass-entry"sv, {} },
		std::pair<string_view, mapped_load> { "mass-entry"sv, {} },
		std::pair<string_view, mapped_load> { "mass-entry"sv, {} },
		std::pair<string_view, mapped_load> { "last-entry"sv, {} }
	);
}

constexpr auto map = test_gen_map();
constexpr auto hash_map = test_gen_hash_map();
constexpr auto mass_hash_map = test_gen_mass_hash_map();

namespace ReflectModule {
	TEST(CompileMap, Find) {

		constexpr auto cc = map.find("c"sv) != map.end();
		constexpr auto cg = map.find("g"sv) != map.end();

		EXPECT_TRUE(cc);
		EXPECT_FALSE(cg);

		constexpr auto c0 = (map.begin() + 0)->data.first <= (map.begin() + 1)->data.first;
		constexpr auto c1 = (map.begin() + 1)->data.first <= (map.begin() + 2)->data.first;
		constexpr auto c2 = (map.begin() + 2)->data.first <= (map.begin() + 3)->data.first;
		constexpr auto c3 = (map.begin() + 3)->data.first <= (map.begin() + 4)->data.first;
		constexpr auto c4 = (map.begin() + 4)->data.first <= (map.begin() + 5)->data.first;

		EXPECT_TRUE(c0);
		EXPECT_TRUE(c1);
		EXPECT_TRUE(c2);
		EXPECT_TRUE(c3);
		EXPECT_TRUE(c4);
	}

	TEST(CompileMap, Contains) { {
			constexpr auto result = map.contains("c"sv);
			EXPECT_TRUE(result);
		} {
			constexpr auto result = map.contains("y"sv);
			EXPECT_TRUE(result);
		} {
			constexpr auto result = map.contains("x"sv);
			EXPECT_FALSE(result);
		}

	}

	TEST(CompileMap, Count) { {
			constexpr auto result = map.count("c"sv);
			EXPECT_EQ(result, 1);
		} {
			constexpr auto result = map.count("x"sv);
			EXPECT_EQ(result, 0);
		} {
			constexpr auto result = map.count("y"sv);
			EXPECT_EQ(result, 2);
		}

	}

	TEST(CompileHashMap, Find) {

		constexpr auto cc = hash_map.find("c"sv) != hash_map.end();
		constexpr auto cg = hash_map.find("g"sv) != hash_map.end();

		EXPECT_TRUE(cc);
		EXPECT_FALSE(cg);

		constexpr auto c0 = (hash_map.begin() + 0)->data.first <= (hash_map.begin() + 1)->data.first;
		constexpr auto c1 = (hash_map.begin() + 1)->data.first <= (hash_map.begin() + 2)->data.first;
		constexpr auto c2 = (hash_map.begin() + 2)->data.first <= (hash_map.begin() + 3)->data.first;
		constexpr auto c3 = (hash_map.begin() + 3)->data.first <= (hash_map.begin() + 4)->data.first;
		constexpr auto c4 = (hash_map.begin() + 4)->data.first <= (hash_map.begin() + 5)->data.first;

		EXPECT_TRUE(c0);
		EXPECT_TRUE(c1);
		EXPECT_TRUE(c2);
		EXPECT_TRUE(c3);
		EXPECT_TRUE(c4);
	}

	TEST(CompileHashMap, MassFind) {

		constexpr auto ffe = mass_hash_map.find("first-entry"sv) != mass_hash_map.end();
		constexpr auto fme = mass_hash_map.find("mass-entry"sv) != mass_hash_map.end();
		constexpr auto fle = mass_hash_map.find("last-entry"sv) != mass_hash_map.end();

		EXPECT_TRUE(ffe);
		EXPECT_TRUE(fme);
		EXPECT_TRUE(fle);
	}

	TEST(CompileHashMap, Contains) { {
			constexpr auto result = hash_map.contains("c"sv);
			EXPECT_TRUE(result);
		} {
			constexpr auto result = hash_map.contains("y"sv);
			EXPECT_TRUE(result);
		} {
			constexpr auto result = hash_map.contains("x"sv);
			EXPECT_FALSE(result);
		}

	}

	TEST(CompileHashMap, Count) { {
			constexpr auto result = hash_map.count("c"sv);
			EXPECT_EQ(result, 1);
		} {
			constexpr auto result = hash_map.count("x"sv);
			EXPECT_EQ(result, 0);
		} {
			constexpr auto result = hash_map.count("y"sv);
			EXPECT_EQ(result, 2);
		}

	}

	TEST(CompileHashMap, MassCount) { {
			constexpr auto result = mass_hash_map.count("first-entry"sv);
			EXPECT_EQ(result, 1);
		} {
			constexpr auto result = mass_hash_map.count("mass-entry"sv);
			EXPECT_EQ(result, 55);
		} {
			constexpr auto result = mass_hash_map.count("last-entry"sv);
			EXPECT_EQ(result, 1);
		}
	}
}
