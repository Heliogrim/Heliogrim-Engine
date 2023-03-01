#include "pch.h"

#include <Engine.Reflect/HeliogrimReflect.hpp>

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
