#include "pch.h"

#include <Engine.Common/Collection/Array.hpp>
#include <Engine.Assets/Types/Asset.hpp>
#include <Engine.Serialization/Layout/DataLayout.hpp>
#include <Engine.Serialization/Layout/LayoutDefine.hpp>
#include <Engine.Serialization/Layout/LayoutDefineSpan.hpp>
#include <Engine.Serialization/Layout/LayoutDefineValue.hpp>
#include <Engine.Serialization/Archive/BufferArchive.hpp>
#include <Engine.Serialization/Archive/LayoutArchive.hpp>

using namespace ember::engine::serialization;
using namespace ember;

TEST(__DummyTest__, Exists) {
    EXPECT_TRUE(true);
}

namespace SerializationModule {

    template <typename ValueType_>
    void simpleBufferTest() {

        BufferArchive archive {};

        using type = ValueType_;
        const type testValue { static_cast<type>(0xA2A2'A2A2'A2A2'A2A2) };

        type write { testValue };
        archive << write;

        archive.seek(0);

        type read { 0 };
        archive >> read;

        EXPECT_EQ(testValue, read);
    }

    TEST(BufferArchive, ReadWriteByte) {
        simpleBufferTest<u8>();
    }

    TEST(BufferArchive, ReadWriteU16) {
        simpleBufferTest<u16>();
    }

    TEST(BufferArchive, ReadWriteU32) {
        simpleBufferTest<u32>();
    }

    TEST(BufferArchive, ReadWriteU64) {
        simpleBufferTest<u64>();
    }

    TEST(BufferArchive, ReadWriteFloat) {
        simpleBufferTest<float>();
    }

    TEST(BufferArchive, ReadWriteDouble) {
        simpleBufferTest<double>();
    }

    TEST(BufferArchive, ReadWriteBool) {

        BufferArchive archive {};

        bool write { false };
        archive << write;

        archive.seek(0);

        bool read { true };
        archive >> read;

        EXPECT_FALSE(read);
        archive.seek(0);

        /**/

        write = true;
        archive << write;

        archive.seek(0);

        read = false;
        archive >> read;

        EXPECT_TRUE(read);
    }

    enum class EnumValueType {
        eEntryZero,
        eEntryOne,
        eEntryTwo
    };

    enum class EnumValueU8 : u8 {
        eEntryZero = 0x0ui8,
        eEntryOne = 0x1ui8,
        eEntryTwo = 0x2ui8
    };

    enum class EnumValueU16 : u16 {
        eEntryZero = 0xFF00ui16,
        eEntryOne = 0xFF01ui16,
        eEntryTwo = 0xFF02ui16
    };

    enum class EnumValueU32 : u32 {
        eEntryZero = 0xFFFF'0000ui32,
        eEntryOne = 0xFFFF'0001ui32,
        eEntryTwo = 0xFFFF'0002ui32
    };

    enum class EnumValueU64 : u64 {
        eEntryZero = 0xFFFF'0000'0000'0000ui64,
        eEntryOne = 0xFFFF'0000'0000'0001ui64,
        eEntryTwo = 0xFFFF'0000'0000'0002ui64
    };

    template <typename EnumType>
    void simpleEnumTest() {

        BufferArchive archive {};

        using type = EnumType;

        type write { EnumType::eEntryTwo };
        archive << write;

        archive.seek(0);

        type read { EnumType::eEntryZero };
        archive >> read;

        EXPECT_EQ(read, EnumType::eEntryTwo);
        archive.seek(0);

        /**/

        type writeN[3] { EnumType::eEntryOne, EnumType::eEntryTwo, EnumType::eEntryZero };
        archive << writeN[0];
        archive << writeN[1];
        archive << writeN[2];

        archive.seek(0);
        read = EnumType::eEntryZero;

        archive >> read;
        EXPECT_EQ(read, EnumType::eEntryOne);

        archive >> read;
        EXPECT_EQ(read, EnumType::eEntryTwo);

        archive >> read;
        EXPECT_EQ(read, EnumType::eEntryZero);

    }

    TEST(BufferArchive, ReadWriteEnumType) {
        simpleEnumTest<EnumValueType>();
    }

    TEST(BufferArchive, ReadWriteEnumU8) {
        simpleEnumTest<EnumValueU8>();
    }

    TEST(BufferArchive, ReadWriteEnumU16) {
        simpleEnumTest<EnumValueU16>();
    }

    TEST(BufferArchive, ReadWriteEnumU32) {
        simpleEnumTest<EnumValueU32>();
    }

    TEST(BufferArchive, ReadWriteEnumU64) {
        simpleEnumTest<EnumValueU64>();
    }

    class TestSerialAsset :
        public ::ember::engine::assets::Asset {
    public:
        template <typename>
        friend class ::ember::engine::serialization::DataLayout;

    public:
        inline static constexpr asset_type_id typeId { "TestSerialAsset"_typeId };

    public:
        TestSerialAsset() :
            Asset({}, typeId) {}

    public:
        u8 t0 = 0x0;
        u16 t1 = 0x0;
        u32 t2 = 0x0;
        u64 t3 = 0x0;
        s8 t4 = -0x0;
        s16 t5 = 0x0;
        s32 t6 = 0x0;
        s64 t7 = 0x0;
        float t8 = 0.F;
        double t9 = 0.;
        // TODO: EnumValueType t10 = EnumValueType::eEntryTwo;
        EnumValueU8 t10 = EnumValueU8::eEntryZero;
    };

}

namespace ember::engine::serialization {
    template <>
    void DataLayout<SerializationModule::TestSerialAsset>::describe() {

        using namespace ::SerializationModule;
        using namespace ::ember::engine::serialization::layout;

        defineValue<LayoutDefineValueType::eUInt8>(offsetof(TestSerialAsset, t0));
        defineValue<LayoutDefineValueType::eUInt16>(offsetof(TestSerialAsset, t1));
        defineValue<LayoutDefineValueType::eUInt32>(offsetof(TestSerialAsset, t2));
        defineValue<LayoutDefineValueType::eUInt64>(offsetof(TestSerialAsset, t3));
        defineValue<LayoutDefineValueType::eInt8>(offsetof(TestSerialAsset, t4));
        defineValue<LayoutDefineValueType::eInt16>(offsetof(TestSerialAsset, t5));
        defineValue<LayoutDefineValueType::eInt32>(offsetof(TestSerialAsset, t6));
        defineValue<LayoutDefineValueType::eInt64>(offsetof(TestSerialAsset, t7));
        defineValue<LayoutDefineValueType::eFloat>(offsetof(TestSerialAsset, t8));
        defineValue<LayoutDefineValueType::eDouble>(offsetof(TestSerialAsset, t9));
        // TODO: defineValue<LayoutDefineValueType::eEnum>(offsetof(TestSerialAsset, t10));
        defineValue<LayoutDefineValueType::eUInt8>(offsetof(TestSerialAsset, t10));
    }
}

namespace SerializationModule {

    TEST(LayoutArchive, SimpleReadWrite) {

        BufferArchive archive {};
        DataLayout<TestSerialAsset> layout {};
        layout.describe();

        TypedLayoutArchive<TestSerialAsset> arch { &archive, &layout };

        auto writeAsset = EmberObject::create<TestSerialAsset>();

        writeAsset->t0 = 0x1;
        writeAsset->t1 = 0x2;
        writeAsset->t2 = 0x3;
        writeAsset->t3 = 0x4;
        writeAsset->t4 = -0x1;
        writeAsset->t5 = -0x2;
        writeAsset->t6 = -0x3;
        writeAsset->t7 = -0x4;
        writeAsset->t8 = 2.345678F;
        writeAsset->t9 = -8.765432;
        // TODO: writeAsset->t10 = EnumValueType::eEntryTwo;
        writeAsset->t10 = EnumValueU8::eEntryTwo;

        arch << writeAsset;

        archive.seek(0);

        auto readAsset = EmberObject::create<TestSerialAsset>();
        arch >> readAsset;

        EXPECT_EQ(readAsset->t0, 0x1);
        EXPECT_EQ(readAsset->t1, 0x2);
        EXPECT_EQ(readAsset->t2, 0x3);
        EXPECT_EQ(readAsset->t3, 0x4);
        EXPECT_EQ(readAsset->t4, -0x1);
        EXPECT_EQ(readAsset->t5, -0x2);
        EXPECT_EQ(readAsset->t6, -0x3);
        EXPECT_EQ(readAsset->t7, -0x4);
        EXPECT_FLOAT_EQ(readAsset->t8, 2.345678F);
        EXPECT_DOUBLE_EQ(readAsset->t9, -8.765432);
        EXPECT_EQ(readAsset->t10, EnumValueU8::eEntryTwo);

        EmberObject::destroy(_STD move(writeAsset));
        EmberObject::destroy(_STD move(readAsset));
    }

    class TestSerialDataBaseAsset :
        public ::ember::engine::assets::Asset {
    public:
        template <typename>
        friend class ::ember::engine::serialization::DataLayout;

    public:
        inline static constexpr asset_type_id typeId { "TestSerialDataBaseAsset"_typeId };

    public:
        TestSerialDataBaseAsset() :
            Asset({}, typeId) {}

    public:
        void setGuid(asset_guid guid_) {
            _guid = guid_;
        }

        void setType(asset_type_id type_) {
            _type = type_;
        }
    };

}

namespace ember::engine::serialization {

    template <>
    void DataLayout<SerializationModule::TestSerialDataBaseAsset>::describe() {

        using namespace ::SerializationModule;
        using namespace ::ember::engine::serialization::layout;

        defineValue<LayoutDefineValueType::eUInt64>(offsetof(TestSerialDataBaseAsset, _guid));
        defineValue<LayoutDefineValueType::eUInt64>(offsetof(TestSerialDataBaseAsset, _type));
    }

}

namespace SerializationModule {

    TEST(LayoutArchive, DataBase) {

        BufferArchive archive {};
        DataLayout<TestSerialDataBaseAsset> layout {};
        layout.describe();

        TypedLayoutArchive<TestSerialDataBaseAsset> arch { &archive, &layout };

        constexpr auto testGuid = asset_guid { 0x2356ui16, 0x12, 0x56, 0x68537136ui32 };
        constexpr auto testType = asset_type_id { "TestBaseAsset_Changed"_typeId };

        auto writeAsset = EmberObject::create<TestSerialDataBaseAsset>();
        writeAsset->setGuid(testGuid);
        writeAsset->setType(testType);
        arch << writeAsset;

        archive.seek(0);

        auto readAsset = EmberObject::create<TestSerialDataBaseAsset>();
        arch >> readAsset;

        EXPECT_EQ(readAsset->get_guid().as_uint64(), testGuid.as_uint64());
        EXPECT_EQ(readAsset->getTypeId(), testType);

        EmberObject::destroy(_STD move(writeAsset));
        EmberObject::destroy(_STD move(readAsset));
    }

    struct TestSubTypePayload {
        u64 p0;
        u64 p1;
        float f0;
        float f1;
    };

    class TestSerialSubTypeAsset :
        public ::ember::engine::assets::Asset {
    public:
        template <typename>
        friend class ::ember::engine::serialization::DataLayout;

    public:
        inline static constexpr asset_type_id typeId { "TestSerialSubTypeAsset"_typeId };

    public:
        TestSerialSubTypeAsset() :
            Asset({}, typeId) {}

    public:
        TestSubTypePayload payload;
    };

}

namespace ember::engine::serialization {

    template <>
    void DataLayout<SerializationModule::TestSubTypePayload>::describe() {

        using namespace ::SerializationModule;
        using namespace ::ember::engine::serialization::layout;

        // TODO: make define for sub objects
        defineValue<LayoutDefineValueType::eUInt64>(offsetof(TestSubTypePayload, p0));
        defineValue<LayoutDefineValueType::eUInt64>(offsetof(TestSubTypePayload, p1));
        defineValue<LayoutDefineValueType::eFloat>(offsetof(TestSubTypePayload, f0));
        defineValue<LayoutDefineValueType::eFloat>(offsetof(TestSubTypePayload, f1));
    }

    template <>
    void DataLayout<SerializationModule::TestSerialSubTypeAsset>::describe() {

        using namespace ::SerializationModule;
        using namespace ::ember::engine::serialization::layout;

        const auto subLayout { make_sptr<DataLayout<TestSubTypePayload>>() };
        subLayout->reflect().storeType<TestSubTypePayload>();
        subLayout->describe();

        defineObject(offsetof(TestSerialSubTypeAsset, payload), subLayout);

    }

}

namespace SerializationModule {

    TEST(LayoutArchive, SimpleSubType) {

        BufferArchive archive {};
        DataLayout<TestSerialSubTypeAsset> layout {};
        layout.describe();

        TypedLayoutArchive<TestSerialSubTypeAsset> arch { &archive, &layout };

        auto writeAsset = EmberObject::create<TestSerialSubTypeAsset>();
        auto readAsset = EmberObject::create<TestSerialSubTypeAsset>();

        /**/

        writeAsset->payload.p0 = 16236267ui64;
        writeAsset->payload.p1 = 2376856ui64;
        writeAsset->payload.f0 = 7896316.135F;
        writeAsset->payload.f1 = 32896.236F;

        /**/

        arch << writeAsset;
        archive.seek(0);
        arch >> readAsset;

        EXPECT_EQ(readAsset->payload.p0, 16236267ui64);
        EXPECT_EQ(readAsset->payload.p1, 2376856ui64);
        EXPECT_EQ(readAsset->payload.f0, 7896316.135F);
        EXPECT_EQ(readAsset->payload.f1, 32896.236F);

        EmberObject::destroy(_STD move(writeAsset));
        EmberObject::destroy(_STD move(readAsset));
    }

    class TestSerialSubTypeSpanAsset :
        public ::ember::engine::assets::Asset {
    public:
        template <typename>
        friend class ::ember::engine::serialization::DataLayout;

    public:
        inline static constexpr asset_type_id typeId { "TestSerialSubTypeSpanAsset"_typeId };

    public:
        TestSerialSubTypeSpanAsset() :
            Asset({}, typeId) {}

    public:
        Array<TestSubTypePayload, 5> payload;
    };

}

namespace ember::engine::serialization {

    template <>
    void DataLayout<SerializationModule::TestSerialSubTypeSpanAsset>::describe() {

        using namespace ::SerializationModule;
        using namespace ::ember::engine::serialization::layout;

        const auto subLayout { make_sptr<DataLayout<TestSubTypePayload>>() };
        subLayout->reflect().storeType<TestSubTypePayload>();
        subLayout->describe();

        defineSpan(offsetof(TestSerialSubTypeSpanAsset, payload), subLayout, 5ui64);

    }

}

namespace SerializationModule {

    TEST(LayoutArchive, SpanSubType) {

        BufferArchive archive {};
        DataLayout<TestSerialSubTypeSpanAsset> layout {};
        layout.describe();

        TypedLayoutArchive<TestSerialSubTypeSpanAsset> arch { &archive, &layout };

        auto writeAsset = EmberObject::create<TestSerialSubTypeSpanAsset>();
        writeAsset->payload[0] = { 32ui64 };
        writeAsset->payload[1] = { 754ui64 };
        writeAsset->payload[2] = { 16ui64 };
        writeAsset->payload[3] = { 5267ui64 };
        writeAsset->payload[4] = { 1ui64 };
        arch << writeAsset;

        archive.seek(0);

        auto readAsset = EmberObject::create<TestSerialSubTypeSpanAsset>();
        arch >> readAsset;

        EXPECT_EQ(writeAsset->payload.size(), readAsset->payload.size());
        EXPECT_EQ(readAsset->payload.front().p0, 32ui64);
        EXPECT_EQ(readAsset->payload[1].p0, 754ui64);
        EXPECT_EQ(readAsset->payload[2].p0, 16ui64);
        EXPECT_EQ(readAsset->payload[3].p0, 5267ui64);
        EXPECT_EQ(readAsset->payload.back().p0, 1ui64);

        EmberObject::destroy(_STD move(writeAsset));
        EmberObject::destroy(_STD move(readAsset));
    }

    class TestSerialSubTypeSliceAsset :
        public ::ember::engine::assets::Asset {
    public:
        template <typename>
        friend class ::ember::engine::serialization::DataLayout;

    public:
        inline static constexpr asset_type_id typeId { "TestSerialSubTypeSliceAsset"_typeId };

    public:
        TestSerialSubTypeSliceAsset() :
            Asset({}, typeId) {}

    public:
        _STD list<TestSubTypePayload> payload;
    };

}

namespace ember::engine::serialization {

    template <>
    void DataLayout<SerializationModule::TestSerialSubTypeSliceAsset>::describe() {

        using namespace ::SerializationModule;
        using namespace ::ember::engine::serialization::layout;

        const auto subLayout { make_sptr<DataLayout<TestSubTypePayload>>() };
        subLayout->reflect().storeType<TestSubTypePayload>();
        subLayout->describe();

        // TODO: make define for sub objects
        defineSlice<_STD list<TestSubTypePayload>>(offsetof(TestSerialSubTypeSliceAsset, payload), subLayout);
    }

}

namespace SerializationModule {

    TEST(LayoutArchive, SliceSubType) {

        BufferArchive archive {};
        DataLayout<TestSerialSubTypeSliceAsset> layout {};
        layout.describe();

        TypedLayoutArchive<TestSerialSubTypeSliceAsset> arch { &archive, &layout };

        auto writeAsset = EmberObject::create<TestSerialSubTypeSliceAsset>();
        writeAsset->payload.push_back({ 32ui64 });
        writeAsset->payload.push_back({ 754ui64 });
        writeAsset->payload.push_back({ 16ui64 });
        writeAsset->payload.push_back({ 5267ui64 });
        writeAsset->payload.push_back({ 1ui64 });
        arch << writeAsset;

        archive.seek(0);

        auto readAsset = EmberObject::create<TestSerialSubTypeSliceAsset>();
        arch >> readAsset;

        EXPECT_EQ(writeAsset->payload.size(), readAsset->payload.size());
        EXPECT_EQ(readAsset->payload.begin()->p0, 32ui64);
        EXPECT_EQ((++readAsset->payload.begin())->p0, 754ui64);
        EXPECT_EQ((++(++readAsset->payload.begin()))->p0, 16ui64);
        EXPECT_EQ((++(++(++readAsset->payload.begin())))->p0, 5267ui64);
        EXPECT_EQ(readAsset->payload.back().p0, 1ui64);

        EmberObject::destroy(_STD move(writeAsset));
        EmberObject::destroy(_STD move(readAsset));
    }

    class TestSerialSubTypeVectorizedSliceAsset :
        public ::ember::engine::assets::Asset {
    public:
        template <typename>
        friend class ::ember::engine::serialization::DataLayout;

    public:
        inline static constexpr asset_type_id typeId { "TestSerialSubTypeVectorizedSliceAsset"_typeId };

    public:
        TestSerialSubTypeVectorizedSliceAsset() :
            Asset({}, typeId) {}

    public:
        Vector<TestSubTypePayload> payload;
    };

}

namespace ember::engine::serialization {

    template <>
    void DataLayout<SerializationModule::TestSerialSubTypeVectorizedSliceAsset>::describe() {

        using namespace ::SerializationModule;
        using namespace ::ember::engine::serialization::layout;

        const auto subLayout { make_sptr<DataLayout<TestSubTypePayload>>() };
        subLayout->reflect().storeType<TestSubTypePayload>();
        subLayout->describe();

        // TODO: make define for sub objects
        defineSlice<Vector<TestSubTypePayload>>(offsetof(TestSerialSubTypeVectorizedSliceAsset, payload), subLayout);
    }

}

namespace SerializationModule {

    TEST(LayoutArchive, VectorizedSliceSubType) {

        BufferArchive archive {};
        DataLayout<TestSerialSubTypeVectorizedSliceAsset> layout {};
        layout.describe();

        TypedLayoutArchive<TestSerialSubTypeVectorizedSliceAsset> arch { &archive, &layout };

        auto writeAsset = EmberObject::create<TestSerialSubTypeVectorizedSliceAsset>();
        writeAsset->payload.resize(200'000'000, { 1231ui64, 738956ui64, 2.32906F, 9230.35F });

        const auto writeStart { _STD chrono::high_resolution_clock::now() };
        arch << writeAsset;
        const auto writeEnd { _STD chrono::high_resolution_clock::now() };

        archive.seek(0);

        auto readAsset = EmberObject::create<TestSerialSubTypeVectorizedSliceAsset>();

        const auto readStart { _STD chrono::high_resolution_clock::now() };
        arch >> readAsset;
        const auto readEnd { _STD chrono::high_resolution_clock::now() };

        EXPECT_EQ(writeAsset->payload.size(), readAsset->payload.size());
        EXPECT_EQ(writeAsset->payload.capacity(), readAsset->payload.capacity());

        /**/

        const auto memCpyStart { _STD chrono::high_resolution_clock::now() };
        Vector<TestSubTypePayload> memCpyTarget {};
        memCpyTarget.resize(writeAsset->payload.size());
        memcpy(memCpyTarget.data(), writeAsset->payload.data(),
            writeAsset->payload.size() * sizeof(TestSubTypePayload));
        const auto memCpyEnd { _STD chrono::high_resolution_clock::now() };

        /**/

        memCpyTarget.clear();
        memCpyTarget.shrink_to_fit();

        /**/

        const auto cpyStart { _STD chrono::high_resolution_clock::now() };
        memCpyTarget.reserve(writeAsset->payload.size());
        for (const auto& entry : writeAsset->payload) {
            memCpyTarget.push_back(entry);
        }
        const auto cpyEnd { _STD chrono::high_resolution_clock::now() };

        /**/

        _STD cout << "Serialization: " <<
            _STD chrono::duration_cast<_STD chrono::milliseconds>(writeEnd - writeStart) <<
            " | Deserialization: " <<
            _STD chrono::duration_cast<_STD chrono::milliseconds>(readEnd - readStart) <<
            " | MemCpy: " <<
            _STD chrono::duration_cast<_STD chrono::milliseconds>(memCpyEnd - memCpyStart) <<
            " | Cpy: " <<
            _STD chrono::duration_cast<_STD chrono::milliseconds>(cpyEnd - cpyStart) <<
            _STD endl;

        EmberObject::destroy(_STD move(writeAsset));
        EmberObject::destroy(_STD move(readAsset));
    }

    class TestSerialSubTypeStringAsset :
        public ::ember::engine::assets::Asset {
    public:
        template <typename>
        friend class ::ember::engine::serialization::DataLayout;

    public:
        inline static constexpr asset_type_id typeId { "TestSerialSubTypeStringAsset"_typeId };

    public:
        TestSerialSubTypeStringAsset() :
            Asset({}, typeId) {}

    public:
        string payload;
    };

}

namespace ember::engine::serialization {

    template <>
    void DataLayout<SerializationModule::TestSerialSubTypeStringAsset>::describe() {

        using namespace ::SerializationModule;
        using namespace ::ember::engine::serialization::layout;

        const auto subLayout { make_sptr<DataLayout<u8>>() };
        subLayout->reflect().storeType<u8>();
        subLayout->describe();

        // TODO: make define for sub objects
        defineSlice<string>(offsetof(TestSerialSubTypeStringAsset, payload), subLayout);
    }

}

namespace SerializationModule {

    TEST(LayoutArchive, StringSubType) {

        BufferArchive archive {};
        DataLayout<TestSerialSubTypeStringAsset> layout {};
        layout.describe();

        TypedLayoutArchive<TestSerialSubTypeStringAsset> arch { &archive, &layout };

        auto writeAsset = EmberObject::create<TestSerialSubTypeStringAsset>();
        writeAsset->payload = "I'm just a string test.";
        arch << writeAsset;

        archive.seek(0);

        auto readAsset = EmberObject::create<TestSerialSubTypeStringAsset>();
        arch >> readAsset;

        EXPECT_EQ(writeAsset->payload.size(), readAsset->payload.size());
        EXPECT_STREQ(writeAsset->payload.c_str(), readAsset->payload.c_str());

        EmberObject::destroy(_STD move(writeAsset));
        EmberObject::destroy(_STD move(readAsset));
    }

}
