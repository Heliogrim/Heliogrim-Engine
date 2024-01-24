#include "pch.h"

#include <Engine.Common/Meta/Constexpr.hpp>
#include <Engine.Reflect/Compile/Map.hpp>
#include <Engine.Reflect/Compile/HashMap.hpp>
#include <Engine.Reflect/CompileTypeId.hpp>
#include <Engine.Reflect/Meta/TypedMetaClass.hpp>
#include <Engine.Reflect/Meta/TypeId.hpp>
#include <Engine.Reflect/Inherit/InheritBase.hpp>
#include <Engine.Reflect/Inherit/InheritMeta.hpp>

using namespace hg;

TEST(__DummyTest__, Exists) {
    EXPECT_TRUE(true);
}

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
        EXPECT_EQ(reflect::typeId<TestDerived01F>(), obj->getMetaClass()->typeId());

        // Outer Class Check
        EXPECT_FALSE(obj->getMetaClass()->is<TestDerived02F>());
        EXPECT_FALSE(obj->getMetaClass()->exact<TestDerived02F>());
        EXPECT_NE(reflect::typeId<TestDerived02F>(), obj->getMetaClass()->typeId());

        // Base Class Check
        //EXPECT_TRUE(obj->getMetaClass()->isType<TestBase01>());
        EXPECT_FALSE(obj->getMetaClass()->exact<TestBase01>());
        EXPECT_NE(reflect::typeId<TestBase01>(), obj->getMetaClass()->typeId());

        //
        delete obj;
    }
}

namespace ReflectModule {
    class TestMetaBase : public InheritBase<TestMetaBase> {
    public:
        TestMetaBase() :
            InheritBase<TestMetaBase>() {}
    };

    class TestMetaDerived : public InheritMeta<TestMetaDerived, TestMetaBase> {
    public:
        TestMetaDerived() = default;
    };

    class TestExtBase {
    public:
        TestExtBase() = default;
    };

    class TestMetaPolyDerived : public InheritMeta<TestMetaPolyDerived, TestMetaBase, TestExtBase> {
    public:
        TestMetaPolyDerived() = default;
    };

    class TestMetaBase2 : public InheritBase<TestMetaBase2> {
    public:
        TestMetaBase2() :
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
        auto obj2 = TestExtBase();

        auto obj3 = TestMetaDerived();

        /**/

        EXPECT_NE(obj0.getMetaClass()->typeId(), obj1.getMetaClass()->typeId());
        EXPECT_TRUE(obj0.getMetaClass()->inherits(TestMetaBase::meta_class::get()));

        EXPECT_TRUE(obj0.getMetaClass()->inherits(ctid<TestExtBase>()));
    }

    TEST(MetaInherit, TestPolyMetaDerived) {

        /**/
        auto obj0 = TestPolyMetaDerived();
        auto obj1 = TestMetaBase();
        auto obj2 = TestMetaBase2();

        auto obj3 = TestMetaDerived();

        /**/

        EXPECT_NE(obj0.getMetaClass()->typeId(), obj1.getMetaClass()->typeId());
        EXPECT_TRUE(obj0.getMetaClass()->inherits(TestMetaBase::meta_class::get()));

        EXPECT_NE(obj0.getMetaClass()->typeId(), obj2.getMetaClass()->typeId());
        EXPECT_TRUE(obj0.getMetaClass()->inherits(TestMetaBase2::meta_class::get()));
    }

    TEST(MetaInherit, DeepInherit) {

        /**/
        auto obj0 = TestPolyMetaPolyDerived();
        auto obj1 = TestMetaPolyDerived();

        auto obj2 = TestMetaBase();
        auto obj3 = TestExtBase();

        /**/

        EXPECT_NE(obj0.getMetaClass()->typeId(), obj1.getMetaClass()->typeId());
        EXPECT_TRUE(obj0.getMetaClass()->inherits(TestMetaPolyDerived::meta_class::get()));

        EXPECT_NE(obj0.getMetaClass()->typeId(), obj2.getMetaClass()->typeId());
        EXPECT_TRUE(obj0.getMetaClass()->inherits(TestMetaBase::meta_class::get()));

        EXPECT_TRUE(obj0.getMetaClass()->inherits(ctid<TestExtBase>()));
    }

    TEST(MetaInherit, StaticMeta) {

        // TODO: Currently we are bound to already instantiated objects, cause otherwise,
        // TODO:    we can not aggregate the required information.
        // TODO:    In the future, we want to achieve this at compile time.

        // Warning: The following instruction will break the whole reflected structure currently:
        EXPECT_TRUE(
            TestPolyMetaPolyDerived::meta_class::get()->inherits(TestMetaBase::meta_class::get())
        );
    }
}

struct mapped_load {
    u64 marker;
};

constexpr auto test_gen_map() {
    auto tmp = make_compile_map<string_view, mapped_load>(
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
    return tmp;
}

constexpr auto test_gen_hash_map() {
    auto tmp = make_compile_hash_map<string_view, mapped_load>(
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
    return tmp;
}

constexpr auto map = test_gen_map();
constexpr auto hash_map = test_gen_hash_map();

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

    TEST(CompileMap, Contains) {

        {
            constexpr auto result = map.contains("c"sv);
            EXPECT_TRUE(result);
        }

        {
            constexpr auto result = map.contains("y"sv);
            EXPECT_TRUE(result);
        }

        {
            constexpr auto result = map.contains("x"sv);
            EXPECT_FALSE(result);
        }

    }

    TEST(CompileMap, Count) {

        {
            constexpr auto result = map.count("c"sv);
            EXPECT_EQ(result, 1);
        }

        {
            constexpr auto result = map.count("x"sv);
            EXPECT_EQ(result, 0);
        }

        {
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

    TEST(CompileHashMap, Contains) {

        {
            constexpr auto result = hash_map.contains("c"sv);
            EXPECT_TRUE(result);
        }

        {
            constexpr auto result = hash_map.contains("y"sv);
            EXPECT_TRUE(result);
        }

        {
            constexpr auto result = hash_map.contains("x"sv);
            EXPECT_FALSE(result);
        }

    }

    TEST(CompileHashMap, Count) {

        {
            constexpr auto result = hash_map.count("c"sv);
            EXPECT_EQ(result, 1);
        }

        {
            constexpr auto result = hash_map.count("x"sv);
            EXPECT_EQ(result, 0);
        }

        {
            constexpr auto result = hash_map.count("y"sv);
            EXPECT_EQ(result, 2);
        }

    }
}
