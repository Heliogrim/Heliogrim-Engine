#include "pch.h"

#include <Engine.Common/Meta/Constexpr.hpp>
#include <Engine.GFX.Glow/Texture/RevVirtualMarkerTexture.hpp>
#include <Engine.Reflect/Compile/Map.hpp>
#include <Engine.Reflect/Compile/HashMap.hpp>
#include <Engine.Reflect/HeliogrimReflect.hpp>
#include <Engine.Reflect/CompileTypeId.hpp>
#include <Engine.Reflect/Meta/TypedMetaClass.hpp>
#include <Engine.Reflect/Inherit/InheritBase.hpp>
#include <Engine.Reflect/Inherit/InheritMeta.hpp>

using namespace hg;

TEST(__DummyTest__, Exists) {
    EXPECT_TRUE(true);
}

namespace ReflectModule {
    class TestBase01 : public HeliogrimObject {
    public:
        inline constexpr static hg::type_id typeId { "TestBase01"_typeId };
    };

    class TestDerived01 : public TestBase01 {
    public:
        inline constexpr static hg::type_id typeId { "TestDerived01"_typeId };
    };

    class TestDerived01F final : public TestBase01 {
    public:
        inline constexpr static hg::type_id typeId { "TestDerived01F"_typeId };
    };

    class TestDerived02 : public TestDerived01 {
    public:
        inline constexpr static hg::type_id typeId { "TestDerived02"_typeId };
    };

    class TestDerived02F final : public TestDerived01 {
    public:
        inline constexpr static hg::type_id typeId { "TestDerived02F"_typeId };
    };

    TEST(HeliogrimObject, Create) {
        //
        auto obj { HeliogrimObject::create<TestBase01>() };

        //
        EXPECT_NE(obj->getClass(), nullptr);

        //
        delete obj;
    }

    TEST(HeliogrimObject, FinalIdentity) {
        //
        auto obj { HeliogrimObject::create<TestDerived01F>() };

        // Same Class Check
        EXPECT_TRUE(obj->getClass()->isType<TestDerived01F>());
        EXPECT_TRUE(obj->getClass()->isExactType<TestDerived01F>());
        EXPECT_EQ(HeliogrimClass::stid<TestDerived01F>(), obj->getClass()->typeId());

        // Outer Class Check
        EXPECT_FALSE(obj->getClass()->isType<TestDerived02F>());
        EXPECT_FALSE(obj->getClass()->isExactType<TestDerived02F>());
        EXPECT_NE(HeliogrimClass::stid<TestDerived02F>(), obj->getClass()->typeId());

        // Base Class Check
        //EXPECT_TRUE(obj->getClass()->isType<TestBase01>());
        EXPECT_FALSE(obj->getClass()->isExactType<TestBase01>());
        EXPECT_NE(HeliogrimClass::stid<TestBase01>(), obj->getClass()->typeId());

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

template <typename>
struct printer;

template <typename... Types_>
struct printer<reflect::__type_list<Types_...>> {
    constexpr void operator()() const noexcept {
        std::cout << "Types(" << sizeof...(Types_) << ")" << std::endl;
        ((std::cout << Types_::name << std::endl), ...);
    }
};

struct Base : InheritBase<Base> {
    constexpr static const char* name = "Base";
};

struct Base2 : InheritBase<Base> {
    constexpr static const char* name = "Base2";
};

struct Base3 : InheritBase<Base> {
    constexpr static const char* name = "Base3";
};

struct Inter1L0 : InheritMeta<Inter1L0, Base> {
    constexpr static const char* name = "Inter1L0";
};

struct Inter2L0 : InheritMeta<Inter2L0, Base, Base2> {
    constexpr static const char* name = "Inter2L0";
};

struct Inter3L0 : InheritMeta<Inter3L0, Base, Base2, Base3> {
    constexpr static const char* name = "Inter3L0";
};

struct Inter1L1 : InheritMeta<Inter1L1, Inter1L0> {
    constexpr static const char* name = "Inter1L1";
};

struct Inter2L1 : InheritMeta<Inter2L1, Inter2L0> {
    constexpr static const char* name = "Inter2L1";
};

struct Inter3L1 : InheritMeta<Inter3L1, Inter3L0> {
    constexpr static const char* name = "Inter3L1";
};

struct Inter1L2 : InheritMeta<Inter1L2, Inter2L0> {
    constexpr static const char* name = "Inter1L2";
};

struct Inter2L2 : InheritMeta<Inter2L2, Inter1L1, Base3> {
    constexpr static const char* name = "Inter2L2";
};

struct Inter3L2 : InheritMeta<Inter3L2, Inter2L1, Base3> {
    constexpr static const char* name = "Inter3L2";
};

struct Derived : InheritMeta<Derived, Inter1L2> {
    constexpr static const char* name = "Derived";
};

struct Derived2 : InheritMeta<Derived2, Inter2L2> {
    constexpr static const char* name = "Derived2";
};

struct Derived3 : InheritMeta<Derived3, Inter2L2> {
    constexpr static const char* name = "Derived3";
};

struct Derived4 : InheritMeta<Derived4, Inter3L2> {
    constexpr static const char* name = "Derived4";
};

struct meta_load {
    u64 marker;
};

template <typename>
struct gen_map;

template <typename... Types_>
struct gen_map<reflect::__type_list<Types_...>> {
    constexpr auto operator()() const noexcept {
        return make_compile_map<string_view, meta_load>(
            _STD make_pair<string_view, meta_load>(Types_::name, {})...
        );
    }
};

TEST(____NS, ____TS) {

    std::cout << "Derived Printer" << std::endl;
    printer<Derived::__inherit_types> {}();

    std::cout << "Derived2 Printer" << std::endl;
    printer<Derived2::__inherit_types> {}();

    std::cout << "Derived3 Printer" << std::endl;
    printer<Derived3::__inherit_types> {}();

    std::cout << "Derived4 Printer" << std::endl;
    printer<Derived4::__inherit_types> {}();

    // TODO: Move map structure to compile-time -> generate jump-table
    //std::map<const char*, meta_load> jump_table = gen_map<Derived4::__dummy_type_list> {}();
    constexpr auto ct = gen_map<Derived4::__inherit_types> {}();

    EXPECT_TRUE(ct.contains("Inter3L2"sv));
    EXPECT_TRUE(ct.contains("Inter2L0"sv));
    EXPECT_FALSE(ct.contains("Inter1L2"sv));

    /**/

    constexpr auto result = Derived4::meta_class::get()->inherits(ctid<Inter2L1>());
    EXPECT_TRUE(result);

    Derived4 d4 {};
    const Base& bt { d4 };
    EXPECT_TRUE(bt.getMetaClass()->inherits(ctid<Inter2L1>()));
}

struct mapped_load {
    u64 marker;
};

constexpr auto test_gen_map() {
    auto tmp = make_compile_map<string_view, mapped_load>(
        _STD pair<string_view, mapped_load> { "a"sv, {} },
        _STD pair<string_view, mapped_load> { "b"sv, {} },
        _STD pair<string_view, mapped_load> { "c"sv, {} },
        _STD pair<string_view, mapped_load> { "d"sv, {} },
        _STD pair<string_view, mapped_load> { "e"sv, {} },
        _STD pair<string_view, mapped_load> { "f"sv, {} },
        //
        _STD pair<string_view, mapped_load> { "y"sv, {} },
        _STD pair<string_view, mapped_load> { "y"sv, {} }
    );
    return tmp;
}

constexpr auto test_gen_hash_map() {
    auto tmp = make_compile_hash_map<string_view, mapped_load>(
        _STD pair<string_view, mapped_load> { "a"sv, {} },
        _STD pair<string_view, mapped_load> { "b"sv, {} },
        _STD pair<string_view, mapped_load> { "c"sv, {} },
        _STD pair<string_view, mapped_load> { "d"sv, {} },
        _STD pair<string_view, mapped_load> { "e"sv, {} },
        _STD pair<string_view, mapped_load> { "f"sv, {} },
        //
        _STD pair<string_view, mapped_load> { "y"sv, {} },
        _STD pair<string_view, mapped_load> { "y"sv, {} }
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
