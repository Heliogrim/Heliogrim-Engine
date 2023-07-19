#include "pch.h"

#include <Engine.Common/Meta/Constexpr.hpp>
#include <Engine.Reflect/HeliogrimReflect.hpp>
#include <Engine.Reflect/CompileTypeId.hpp>
#include <Engine.Reflect/Meta/TypedMetaClass.hpp>
#include <Engine.Reflect/Inherit/InheritBase.hpp>

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

        // Cleanup
        TypedMetaClass<TestMetaBase>::destroy();
        TypedMetaClass<TestMetaDerived>::destroy();
    }

    TEST(MetaInherit, AdvancedInheritSM) {

        /**/
        auto obj0 = TestMetaPolyDerived();
        auto obj1 = TestMetaBase();
        auto obj2 = TestExtBase();

        auto obj3 = TestMetaDerived();

        /**/

        EXPECT_NE(obj0.getMetaClass()->typeId(), obj1.getMetaClass()->typeId());
        EXPECT_TRUE(obj0.getMetaClass()->inherits(TypedMetaClass<TestMetaBase>::get()));

        EXPECT_TRUE(obj0.getMetaClass()->inherits(ctid<TestExtBase>()));

        /* Cleanup */
        TypedMetaClass<TestMetaDerived>::destroy();
        TypedMetaClass<TestExtBase>::destroy();
        TypedMetaClass<TestMetaBase>::destroy();
        TypedMetaClass<TestMetaPolyDerived>::destroy();
    }

    TEST(MetaInherit, TestPolyMetaDerived) {

        /**/
        auto obj0 = TestPolyMetaDerived();
        auto obj1 = TestMetaBase();
        auto obj2 = TestMetaBase2();

        auto obj3 = TestMetaDerived();

        /**/

        EXPECT_NE(obj0.getMetaClass()->typeId(), obj1.getMetaClass()->typeId());
        EXPECT_TRUE(obj0.getMetaClass()->inherits(TypedMetaClass<TestMetaBase>::get()));

        EXPECT_NE(obj0.getMetaClass()->typeId(), obj2.getMetaClass()->typeId());
        EXPECT_TRUE(obj0.getMetaClass()->inherits(TypedMetaClass<TestMetaBase2>::get()));

        /* Cleanup */
        TypedMetaClass<TestMetaDerived>::destroy();
        TypedMetaClass<TestMetaBase2>::destroy();
        TypedMetaClass<TestMetaBase>::destroy();
        TypedMetaClass<TestPolyMetaDerived>::destroy();
    }

    TEST(MetaInherit, DeepInherit) {

        /**/
        auto obj0 = TestPolyMetaPolyDerived();
        auto obj1 = TestMetaPolyDerived();

        auto obj2 = TestMetaBase();
        auto obj3 = TestExtBase();

        /**/

        EXPECT_NE(obj0.getMetaClass()->typeId(), obj1.getMetaClass()->typeId());
        EXPECT_TRUE(obj0.getMetaClass()->inherits(TypedMetaClass<TestMetaPolyDerived>::get()));

        EXPECT_NE(obj0.getMetaClass()->typeId(), obj2.getMetaClass()->typeId());
        EXPECT_TRUE(obj0.getMetaClass()->inherits(TypedMetaClass<TestMetaBase>::get()));

        EXPECT_TRUE(obj0.getMetaClass()->inherits(ctid<TestExtBase>()));

        /* Cleanup */
        TypedMetaClass<TestExtBase>::destroy();
        TypedMetaClass<TestMetaBase>::destroy();
        TypedMetaClass<TestMetaPolyDerived>::destroy();
        TypedMetaClass<TestPolyMetaPolyDerived>::destroy();
    }

    TEST(MetaInherit, StaticMeta) {

        // TODO: Currently we are bound to already instantiated objects, cause otherwise,
        // TODO:    we can not aggregate the required information.
        // TODO:    In the future, we want to achieve this at compile time.

        // Warning: The following instruction will break the whole reflected structure currently:
        EXPECT_TRUE(
            TypedMetaClass<TestPolyMetaPolyDerived>::get()
            ->inherits(TypedMetaClass<TestMetaBase>::get())
        );
    }
}
