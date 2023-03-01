#include "pch.h"

#include <Engine.Common/Collection/Array.hpp>
#include <Engine.Assets/Types/Asset.hpp>
#include <Engine.Serialization/Layout/DataLayout.hpp>
#include <Engine.Serialization/Layout/LayoutDefine.hpp>
#include <Engine.Serialization/Layout/LayoutDefineSpan.hpp>
#include <Engine.Serialization/Layout/LayoutDefineValue.hpp>
#include <Engine.Serialization/Archive/BufferArchive.hpp>
#include <Engine.Serialization/Archive/LayoutArchive.hpp>
#include <Engine.Serialization/Archive/StructuredArchive.hpp>

#include "Engine.Serialization/Structure/IntegralScopedSlot.hpp"
#include "Engine.Serialization/Structure/MapEntryScopedSlot.hpp"
#include "Engine.Serialization/Structure/MapScopedSlot.hpp"
#include "Engine.Serialization/Structure/ScopedStructureSlot.hpp"
#include "Engine.Serialization/Structure/SeqScopedSlot.hpp"
#include "Engine.Serialization/Structure/SliceScopedSlot.hpp"
#include "Engine.Serialization/Structure/StringScopedSlot.hpp"
#include "Engine.Serialization/Structure/StructScopedSlot.hpp"
#include "Engine.Serialization/Structure/FloatScopedSlot.hpp"

using namespace hg::engine::serialization;
using namespace hg;

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
        public ::hg::engine::assets::Asset {
    public:
        template <typename>
        friend class ::hg::engine::serialization::DataLayout;

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

namespace hg::engine::serialization {
    template <>
    void DataLayout<SerializationModule::TestSerialAsset>::describe() {

        using namespace ::SerializationModule;
        using namespace ::hg::engine::serialization::layout;

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

        auto writeAsset = HeliogrimObject::create<TestSerialAsset>();

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

        auto readAsset = HeliogrimObject::create<TestSerialAsset>();
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

        HeliogrimObject::destroy(_STD move(writeAsset));
        HeliogrimObject::destroy(_STD move(readAsset));
    }

    class TestSerialDataBaseAsset :
        public ::hg::engine::assets::Asset {
    public:
        template <typename>
        friend class ::hg::engine::serialization::DataLayout;

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

namespace hg::engine::serialization {
    template <>
    void DataLayout<SerializationModule::TestSerialDataBaseAsset>::describe() {

        using namespace ::SerializationModule;
        using namespace ::hg::engine::serialization::layout;

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

        auto writeAsset = HeliogrimObject::create<TestSerialDataBaseAsset>();
        writeAsset->setGuid(testGuid);
        writeAsset->setType(testType);
        arch << writeAsset;

        archive.seek(0);

        auto readAsset = HeliogrimObject::create<TestSerialDataBaseAsset>();
        arch >> readAsset;

        EXPECT_EQ(readAsset->get_guid().as_uint64(), testGuid.as_uint64());
        EXPECT_EQ(readAsset->getTypeId(), testType);

        HeliogrimObject::destroy(_STD move(writeAsset));
        HeliogrimObject::destroy(_STD move(readAsset));
    }

    struct TestSubTypePayload {
        u64 p0;
        u64 p1;
        float f0;
        float f1;
    };

    class TestSerialSubTypeAsset :
        public ::hg::engine::assets::Asset {
    public:
        template <typename>
        friend class ::hg::engine::serialization::DataLayout;

    public:
        inline static constexpr asset_type_id typeId { "TestSerialSubTypeAsset"_typeId };

    public:
        TestSerialSubTypeAsset() :
            Asset({}, typeId) {}

    public:
        TestSubTypePayload payload;
    };
}

namespace hg::engine::serialization {
    template <>
    void DataLayout<SerializationModule::TestSubTypePayload>::describe() {

        using namespace ::SerializationModule;
        using namespace ::hg::engine::serialization::layout;

        // TODO: make define for sub objects
        defineValue<LayoutDefineValueType::eUInt64>(offsetof(TestSubTypePayload, p0));
        defineValue<LayoutDefineValueType::eUInt64>(offsetof(TestSubTypePayload, p1));
        defineValue<LayoutDefineValueType::eFloat>(offsetof(TestSubTypePayload, f0));
        defineValue<LayoutDefineValueType::eFloat>(offsetof(TestSubTypePayload, f1));
    }

    template <>
    void DataLayout<SerializationModule::TestSerialSubTypeAsset>::describe() {

        using namespace ::SerializationModule;
        using namespace ::hg::engine::serialization::layout;

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

        auto writeAsset = HeliogrimObject::create<TestSerialSubTypeAsset>();
        auto readAsset = HeliogrimObject::create<TestSerialSubTypeAsset>();

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

        HeliogrimObject::destroy(_STD move(writeAsset));
        HeliogrimObject::destroy(_STD move(readAsset));
    }

    class TestSerialSubTypeSpanAsset :
        public ::hg::engine::assets::Asset {
    public:
        template <typename>
        friend class ::hg::engine::serialization::DataLayout;

    public:
        inline static constexpr asset_type_id typeId { "TestSerialSubTypeSpanAsset"_typeId };

    public:
        TestSerialSubTypeSpanAsset() :
            Asset({}, typeId) {}

    public:
        Array<TestSubTypePayload, 5> payload;
    };
}

namespace hg::engine::serialization {
    template <>
    void DataLayout<SerializationModule::TestSerialSubTypeSpanAsset>::describe() {

        using namespace ::SerializationModule;
        using namespace ::hg::engine::serialization::layout;

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

        auto writeAsset = HeliogrimObject::create<TestSerialSubTypeSpanAsset>();
        writeAsset->payload[0] = { 32ui64 };
        writeAsset->payload[1] = { 754ui64 };
        writeAsset->payload[2] = { 16ui64 };
        writeAsset->payload[3] = { 5267ui64 };
        writeAsset->payload[4] = { 1ui64 };
        arch << writeAsset;

        archive.seek(0);

        auto readAsset = HeliogrimObject::create<TestSerialSubTypeSpanAsset>();
        arch >> readAsset;

        EXPECT_EQ(writeAsset->payload.size(), readAsset->payload.size());
        EXPECT_EQ(readAsset->payload.front().p0, 32ui64);
        EXPECT_EQ(readAsset->payload[1].p0, 754ui64);
        EXPECT_EQ(readAsset->payload[2].p0, 16ui64);
        EXPECT_EQ(readAsset->payload[3].p0, 5267ui64);
        EXPECT_EQ(readAsset->payload.back().p0, 1ui64);

        HeliogrimObject::destroy(_STD move(writeAsset));
        HeliogrimObject::destroy(_STD move(readAsset));
    }

    class TestSerialSubTypeSliceAsset :
        public ::hg::engine::assets::Asset {
    public:
        template <typename>
        friend class ::hg::engine::serialization::DataLayout;

    public:
        inline static constexpr asset_type_id typeId { "TestSerialSubTypeSliceAsset"_typeId };

    public:
        TestSerialSubTypeSliceAsset() :
            Asset({}, typeId) {}

    public:
        _STD list<TestSubTypePayload> payload;
    };
}

namespace hg::engine::serialization {
    template <>
    void DataLayout<SerializationModule::TestSerialSubTypeSliceAsset>::describe() {

        using namespace ::SerializationModule;
        using namespace ::hg::engine::serialization::layout;

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

        auto writeAsset = HeliogrimObject::create<TestSerialSubTypeSliceAsset>();
        writeAsset->payload.push_back({ 32ui64 });
        writeAsset->payload.push_back({ 754ui64 });
        writeAsset->payload.push_back({ 16ui64 });
        writeAsset->payload.push_back({ 5267ui64 });
        writeAsset->payload.push_back({ 1ui64 });
        arch << writeAsset;

        archive.seek(0);

        auto readAsset = HeliogrimObject::create<TestSerialSubTypeSliceAsset>();
        arch >> readAsset;

        EXPECT_EQ(writeAsset->payload.size(), readAsset->payload.size());
        EXPECT_EQ(readAsset->payload.begin()->p0, 32ui64);
        EXPECT_EQ((++readAsset->payload.begin())->p0, 754ui64);
        EXPECT_EQ((++(++readAsset->payload.begin()))->p0, 16ui64);
        EXPECT_EQ((++(++(++readAsset->payload.begin())))->p0, 5267ui64);
        EXPECT_EQ(readAsset->payload.back().p0, 1ui64);

        HeliogrimObject::destroy(_STD move(writeAsset));
        HeliogrimObject::destroy(_STD move(readAsset));
    }

    class TestSerialSubTypeVectorizedSliceAsset :
        public ::hg::engine::assets::Asset {
    public:
        template <typename>
        friend class ::hg::engine::serialization::DataLayout;

    public:
        inline static constexpr asset_type_id typeId { "TestSerialSubTypeVectorizedSliceAsset"_typeId };

    public:
        TestSerialSubTypeVectorizedSliceAsset() :
            Asset({}, typeId) {}

    public:
        Vector<TestSubTypePayload> payload;
    };
}

namespace hg::engine::serialization {
    template <>
    void DataLayout<SerializationModule::TestSerialSubTypeVectorizedSliceAsset>::describe() {

        using namespace ::SerializationModule;
        using namespace ::hg::engine::serialization::layout;

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

        auto writeAsset = HeliogrimObject::create<TestSerialSubTypeVectorizedSliceAsset>();
        //writeAsset->payload.resize(200'000'000, { 1231ui64, 738956ui64, 2.32906F, 9230.35F });
        writeAsset->payload.resize(2'000'000, { 1231ui64, 738956ui64, 2.32906F, 9230.35F });

        const auto writeStart { _STD chrono::high_resolution_clock::now() };
        arch << writeAsset;
        const auto writeEnd { _STD chrono::high_resolution_clock::now() };

        archive.seek(0);

        auto readAsset = HeliogrimObject::create<TestSerialSubTypeVectorizedSliceAsset>();

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

        HeliogrimObject::destroy(_STD move(writeAsset));
        HeliogrimObject::destroy(_STD move(readAsset));
    }

    class TestSerialSubTypeStringAsset :
        public ::hg::engine::assets::Asset {
    public:
        template <typename>
        friend class ::hg::engine::serialization::DataLayout;

    public:
        inline static constexpr asset_type_id typeId { "TestSerialSubTypeStringAsset"_typeId };

    public:
        TestSerialSubTypeStringAsset() :
            Asset({}, typeId) {}

    public:
        string payload;
    };
}

namespace hg::engine::serialization {
    template <>
    void DataLayout<SerializationModule::TestSerialSubTypeStringAsset>::describe() {

        using namespace ::SerializationModule;
        using namespace ::hg::engine::serialization::layout;

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

        auto writeAsset = HeliogrimObject::create<TestSerialSubTypeStringAsset>();
        writeAsset->payload = "I'm just a string test.";
        arch << writeAsset;

        archive.seek(0);

        auto readAsset = HeliogrimObject::create<TestSerialSubTypeStringAsset>();
        arch >> readAsset;

        EXPECT_EQ(writeAsset->payload.size(), readAsset->payload.size());
        EXPECT_STREQ(writeAsset->payload.c_str(), readAsset->payload.c_str());

        HeliogrimObject::destroy(_STD move(writeAsset));
        HeliogrimObject::destroy(_STD move(readAsset));
    }
}

namespace SerializationModule {
    TEST(StructureArchive, IntegralRead) {

        BufferArchive archive {};
        StructuredArchive arch { &archive };

        /**/

        archive << StructureSlotType::eU64;
        archive << 432856789ui64;

        archive.seek(0);

        /**/

        {
            const auto read = arch.getRootSlot();

            u64 dst { 0 };
            read.intoIntegral<u64>() >> dst;

            /**/

            EXPECT_EQ(dst, 432856789ui64);
        }

        /**/

        archive.seek(0);
        archive << StructureSlotType::eU32;
        archive << 267586ui32;
        archive.seek(0);

        /**/

        {
            const auto read = arch.getRootSlot();

            u32 dst { 0 };
            read.intoIntegral<u32>() >> dst;

            /**/

            EXPECT_EQ(dst, 267586ui32);
        }

        /**/

        archive.seek(0);
        archive << StructureSlotType::eU16;
        archive << 24676ui16;
        archive.seek(0);

        /**/

        {
            const auto read = arch.getRootSlot();

            u16 dst { 0 };
            read.intoIntegral<u16>() >> dst;

            /**/

            EXPECT_EQ(dst, 24676ui16);
        }
    }

    TEST(StructureArchive, StringRead) {

        BufferArchive archive {};
        StructuredArchive arch { &archive };

        /**/

        string value { "String Test" };
        archive << StructureSlotType::eString;
        archive << value.size();

        archive.serializeBytes(value.data(), value.size(), ArchiveStreamMode::eIn);

        archive.seek(0);

        /**/

        {
            const auto read = arch.getRootSlot();

            string dst {};
            read.intoString() >> dst;

            /**/

            EXPECT_EQ(dst, "String Test");
        }
    }

    TEST(StructureArchive, StringReadWrite) {

        BufferArchive archive {};
        StructuredArchive arch { &archive };

        const string src { "String Test" };

        {
            auto write = arch.getRootSlot();
            write.intoString() << src;
        }

        /**/

        archive.seek(0);

        /**/

        {
            const auto read = arch.getRootSlot();

            string dst {};
            read.intoString() >> dst;

            /**/

            EXPECT_EQ(dst, src);
        }
    }

    TEST(StructureArchive, EmptyStruct) {

        BufferArchive archive {};
        StructuredArchive arch { &archive };

        {
            auto write = arch.insertRootSlot();
            write.intoStruct();
        }

        /**/

        EXPECT_EQ(archive.size(), 9);
        archive.seek(0);
    }

    TEST(StructureArchive, EmptyRecordStruct) {

        BufferArchive archive {};
        StructuredArchive arch { &archive };

        {
            auto write = arch.insertRootSlot();
            write.intoStruct().insertSlot<void>("test");
        }

        /**/

        EXPECT_EQ(archive.size(), (1 + 8) + (1 + 8 + 4) + (1 + 8));
        archive.seek(0);

        {
            const auto read = arch.getRootSlot();
            read.intoStruct().getSlot<void>("test");
        }
    }

    TEST(StructureArchive, IntegralStructReadWrite) {

        BufferArchive archive {};
        StructuredArchive arch { &archive };

        const u64 src = 62788267ui64;
        {
            auto write = arch.insertRootSlot();
            write.intoStruct().insertSlot<u64>("test") << src;
        }

        /**/

        archive.seek(0);

        /**/

        u64 dst = 0ui64;
        {
            const auto read = arch.getRootSlot();
            read.intoStruct().getSlot<u64>("test") >> dst;
        }

        /**/

        EXPECT_EQ(dst, src);
    }

    TEST(StructureArchive, StringStructReadWrite) {

        BufferArchive archive {};
        StructuredArchive arch { &archive };

        const string src { "Test String" };
        {
            auto write = arch.insertRootSlot();
            write.intoStruct().insertSlot<string>("test") << src;
        }

        /**/

        archive.seek(0);

        /**/

        string dst {};
        {
            const auto read = arch.getRootSlot();
            read.intoStruct().getSlot<string>("test") >> dst;
        }

        /**/

        EXPECT_EQ(dst, src);
    }

    TEST(StructureArchive, NativeStructReadWrite) {

        BufferArchive archive {};
        StructuredArchive arch { &archive };

        const u64 src0 = 26785626ui64;
        const string src1 { "Test String" };
        {
            auto write = arch.insertRootSlot();
            auto slot = write.intoStruct();
            slot.insertSlot<u64>("test0") << src0;
            slot.insertSlot<string>("test1") << src1;
        }

        /**/

        archive.seek(0);

        /**/

        u64 dst0 = 0ui64;
        string dst1 {};
        {
            const auto read = arch.getRootSlot();
            auto slot = read.intoStruct();
            slot.getSlot<u64>("test0") >> dst0;
            slot.getSlot<string>("test1") >> dst1;
        }

        /**/

        EXPECT_EQ(dst0, src0);
        EXPECT_EQ(dst1, src1);
    }

    TEST(StructureArchive, SwappingNativeStructReadWrite) {

        BufferArchive archive {};
        StructuredArchive arch { &archive };

        const u64 src0 = 637861891ui64;
        const string src1 { "Swap String" };
        {
            auto write = arch.insertRootSlot();
            auto slot = write.intoStruct();
            slot.insertSlot<u64>("test0") << src0;
            slot.insertSlot<string>("test1") << src1;
        }

        /**/

        archive.seek(0);

        /**/

        u64 dst0 = 0ui64;
        string dst1 {};
        {
            const auto read = arch.getRootSlot();
            auto slot = read.intoStruct();
            slot.getSlot<string>("test1") >> dst1;
            slot.getSlot<u64>("test0") >> dst0;
        }

        /**/

        EXPECT_EQ(dst0, src0);
        EXPECT_EQ(dst1, src1);
    }

    TEST(StructureArchive, NestedStructReadWrite) {

        BufferArchive archive {};
        StructuredArchive arch { &archive };

        const u64 src = 46262747ui64;
        {
            auto write = arch.insertRootSlot();
            write.intoStruct().insertSlot<void>("test0").intoStruct().insertSlot<u64>("test1") << src;
        }

        /**/

        archive.seek(0);

        /**/

        u64 dst = 0ui64;
        {
            const auto read = arch.getRootSlot();
            read.intoStruct().getSlot<void>("test0").intoStruct().getSlot<u64>("test1") >> dst;
        }

        /**/

        EXPECT_EQ(dst, src);
    }

    TEST(StructureArchive, NestedStructReadWrite2) {

        BufferArchive archive {};
        StructuredArchive arch { &archive };

        const u64 src1 = 126784ui64;
        const u64 src2 = 6312785ui64;
        {
            auto write = arch.insertRootSlot();
            auto struct0 = write.intoStruct();

            auto struct1 = struct0.insertSlot<void>("obj1").intoStruct();
            struct1.insertSlot<u64>("test1") << src1;
            struct1.leaveSlot();

            auto struct2 = struct0.insertSlot<void>("obj2").intoStruct();
            struct2.insertSlot<u64>("test2") << src2;
            struct2.leaveSlot();
        }

        /**/

        archive.seek(0);

        /**/

        u64 dst1 = 0ui64;
        u64 dst2 = 0ui64;
        {
            const auto read = arch.getRootSlot();
            auto struct0 = read.intoStruct();

            auto struct1 = struct0.getSlot<void>("obj1").intoStruct();
            struct1.getSlot<u64>("test1") >> dst1;

            auto struct2 = struct0.getSlot<void>("obj2").intoStruct();
            struct2.getSlot<u64>("test2") >> dst2;
        }

        /**/

        EXPECT_EQ(dst1, src1);
        EXPECT_EQ(dst2, src2);
    }

    TEST(StructureArchive, SliceReadWrite) {

        BufferArchive archive {};
        StructuredArchive arch { &archive };

        const Vector<u64> src { { 2165787ui64, 7826523ui64, 32597698ui64 } };

        {
            auto write = arch.getRootSlot();
            write.intoSlice<u64, Vector>() << src;
        }

        /**/

        archive.seek(0);

        /**/

        {
            const auto read = arch.getRootSlot();

            Vector<u64> dst {};
            read.intoSlice<u64, Vector>() >> dst;

            /**/

            EXPECT_EQ(dst.size(), src.size());

            for (u64 i = 0; i < dst.size(); i++) {
                EXPECT_EQ(dst[i], src[i]);
            }
        }
    }

    TEST(StructureArchive, StringSliceReadWrite) {

        BufferArchive archive {};
        StructuredArchive arch { &archive };

        const Vector<string> src { { "$0::Test String", "$1::Test String", "$2::Test String" } };

        {
            auto write = arch.getRootSlot();
            write.intoSlice<string, Vector>() << src;
        }

        /**/

        archive.seek(0);

        /**/

        {
            const auto read = arch.getRootSlot();

            Vector<string> dst {};
            read.intoSlice<string, Vector>() >> dst;

            /**/

            EXPECT_EQ(dst.size(), src.size());

            for (u64 i = 0; i < dst.size(); i++) {
                EXPECT_EQ(dst[i], src[i]);
            }
        }
    }

    TEST(StructureArchive, MapReadWrite) {

        BufferArchive archive {};
        StructuredArchive arch { &archive };

        _STD map<string, u64> src {
            { "$0::key", 3657876ui64 },
            { "$1::key", 32876797ui64 },
            { "$3::key", 4267198ui64 }
        };

        {
            auto write = arch.getRootSlot();
            write.intoMap<string, u64, _STD map>() << src;
        }

        /**/

        archive.seek(0);

        /**/

        {
            const auto read = arch.getRootSlot();

            _STD map<string, u64> dst {};
            read.intoMap<string, u64, _STD map>() >> dst;

            /**/

            EXPECT_EQ(dst.size(), src.size());

            for (const auto& srcEntry : src) {
                EXPECT_TRUE(dst.contains(srcEntry.first));

                if (dst.contains(srcEntry.first)) {
                    EXPECT_EQ(dst[srcEntry.first], srcEntry.second);
                }
            }
        }
    }

    TEST(StructureArchive, SeqReadWrite) {

        BufferArchive archive {};
        StructuredArchive arch { &archive };

        struct Obj0 {
            string name;
            string surname;

            u16 age;
            s64 money;

            void serialize(mref<RecordScopedSlot> slot_) const {
                auto slot = slot_.intoStruct();

                slot.insertSlot<string>("name") << name;
                slot.insertSlot<string>("surname") << surname;
                slot.insertSlot<u16>("age") << age;
                slot.insertSlot<s64>("money") << money;
            }

            void deserialize(cref<RecordScopedSlot> slot_) {
                auto slot = slot_.intoStruct();

                slot.getSlot<string>("name") >> name;
                slot.getSlot<string>("surname") >> surname;
                slot.getSlot<u16>("age") >> age;
                slot.getSlot<s64>("money") >> money;
            }

            bool operator==(cref<Obj0> other_) const {
                return name == other_.name && surname == other_.surname & age == other_.age && money == other_.money;
            }
        };

        struct Obj1 {
            string name;
            string path;

            float range;
            float dmg;

            s64 cost;

            void serialize(mref<RecordScopedSlot> slot_) const {
                auto slot = slot_.intoStruct();

                slot.insertSlot<string>("name") << name;
                slot.insertSlot<string>("path") << path;
                slot.insertSlot<float>("range") << range;
                slot.insertSlot<float>("dmg") << dmg;
                slot.insertSlot<s64>("cost") << cost;
            }

            void deserialize(cref<RecordScopedSlot> slot_) {
                auto slot = slot_.intoStruct();

                slot.getSlot<string>("name") >> name;
                slot.getSlot<string>("path") >> path;
                slot.getSlot<float>("range") >> range;
                slot.getSlot<float>("dmg") >> dmg;
                slot.getSlot<s64>("cost") >> cost;
            }

            bool operator==(cref<Obj1> other_) const {
                return name == other_.name &&
                    path == other_.path &&
                    range == other_.range &&
                    dmg == other_.dmg &&
                    cost == other_.cost;
            }
        };

        struct Obj2 {
            void serialize(mref<RecordScopedSlot> slot_) const {}

            void deserialize(cref<RecordScopedSlot> slot_) {}

            bool operator==(cref<Obj2> other_) const {
                return true;
            }
        };

        const Obj0 src0 {};
        const Obj1 src1 {};
        const Obj2 src2 {};

        {
            auto write = arch.insertRootSlot();
            auto slot = write.intoSeq();

            src0.serialize(slot.addRecordSlot());
            src1.serialize(slot.addRecordSlot());
            src2.serialize(slot.addRecordSlot());
        }

        /**/

        archive.seek(0);

        /**/

        Obj0 dst0 {};
        Obj1 dst1 {};
        Obj2 dst2 {};

        {
            const auto read = arch.getRootSlot();
            auto slot = read.intoSeq();

            dst0.deserialize(slot.getRecordSlot(0));
            dst1.deserialize(slot.getRecordSlot(1));
            dst2.deserialize(slot.getRecordSlot(2));
        }

        /**/

        EXPECT_EQ(dst0, src0);
        EXPECT_EQ(dst1, src1);
        EXPECT_EQ(dst2, src2);
    }

    TEST(StructureArchive, SimpleReadWrite) {

        BufferArchive archive {};
        StructuredArchive arch { &archive };

        struct TI3Obj {
            string data0;
            u64 data1;

            void serialize(mref<RecordScopedSlot> slot_) {
                auto slot = slot_.intoStruct();
                slot.insertSlot<string>("data0") << data0;
                slot.insertSlot<u64>("data1") << data1;
            }

            void deserialize(cref<RecordScopedSlot> slot_) {
                const auto slot = slot_.intoStruct();
                slot.getSlot<string>("data0") >> data0;
                slot.getSlot<u64>("data1") >> data1;
            }

            [[nodiscard]] bool operator==(cref<TI3Obj> other_) const noexcept {
                if (data0 != other_.data0 || data1 != other_.data1) {
                    return false;
                }
                return true;
            }
        };

        struct TI2Obj {
            string data0;
            u32 data1;
            u32 data2;

            _STD map<string, u64> data3;

            TI3Obj obj0;

            void serialize(mref<RecordScopedSlot> slot_) {
                auto slot = slot_.intoStruct();

                slot.insertSlot<string>("data0") << data0;
                slot.insertSlot<u32>("data1") << data1;
                slot.insertSlot<u32>("data2") << data2;
                slot.insertSlot<string, u64, _STD map>("data3") << data3;

                obj0.serialize(slot.insertSlot<void>("obj0"));
            }

            void deserialize(cref<RecordScopedSlot> slot_) {
                const auto slot = slot_.intoStruct();

                slot.getSlot<string>("data0") >> data0;
                slot.getSlot<u32>("data1") >> data1;
                slot.getSlot<u32>("data2") >> data2;
                slot.getSlot<string, u64, _STD map>("data3") >> data3;

                obj0.deserialize(slot.getSlot<void>("obj0"));
            }

            [[nodiscard]] bool operator==(cref<TI2Obj> other_) const noexcept {
                if (data0 != other_.data0 || data1 != other_.data1 || data2 != other_.data2 || data3 != other_.data3) {
                    return false;
                }
                return (obj0 == other_.obj0);
            }
        };

        struct TI1Obj {
            u64 data0;

            TI2Obj obj0;
            TI2Obj obj1;

            u64 data1;

            _STD vector<string> data2;

            void serialize(mref<RecordScopedSlot> slot_) {
                auto slot = slot_.intoStruct();
                slot.insertSlot<u64>("data0") << data0;

                slot.insertSlot<string>("test") << "test";

                obj0.serialize(slot.insertSlot<void>("obj0"));
                obj1.serialize(slot.insertSlot<void>("obj1"));

                slot.insertSlot<u64>("data1") << data1;
                slot.insertSlot<string, _STD vector>("data2") << data2;
            }

            void deserialize(cref<RecordScopedSlot> slot_) {
                const auto slot = slot_.intoStruct();
                slot.getSlot<u64>("data0") >> data0;

                string test {};
                slot.getSlot<string>("test") >> test;

                obj1.deserialize(slot.getSlot<void>("obj1"));
                obj0.deserialize(slot.getSlot<void>("obj0"));

                slot.getSlot<u64>("data1") >> data1;
                slot.getSlot<string, _STD vector>("data2") >> data2;
            }

            [[nodiscard]] bool operator==(cref<TI1Obj> other_) const noexcept {
                if (data0 != other_.data0 || data1 != other_.data1 || data2 != other_.data2) {
                    return false;
                }
                return (obj0 == other_.obj0) && (obj1 == other_.obj1);
            }
        };

        struct TRObj {
            string data0;
            u64 data1;
            u32 data2;
            _STD vector<u16> data3;

            TI1Obj obj0;

            void serialize(mref<RecordScopedSlot> slot_) {
                auto slot = slot_.intoStruct();
                slot.insertSlot<string>("data0") << data0;
                slot.insertSlot<u64>("data1") << data1;
                slot.insertSlot<u32>("data2") << data2;
                slot.insertSlot<u16, _STD vector>("data3") << data3;

                obj0.serialize(slot.insertSlot<void>("obj0"));
            }

            void deserialize(cref<RecordScopedSlot> slot_) {
                const auto slot = slot_.intoStruct();
                slot.getSlot<string>("data0") >> data0;
                slot.getSlot<u64>("data1") >> data1;
                slot.getSlot<u32>("data2") >> data2;
                slot.getSlot<u16, _STD vector>("data3") >> data3;

                obj0.deserialize(slot.getSlot<void>("obj0"));
            }

            [[nodiscard]] bool operator==(cref<TRObj> other_) const noexcept {
                if (data0 != other_.data0 || data1 != other_.data1 || data2 != other_.data2 || data3 != other_.data3) {
                    return false;
                }
                return obj0 == other_.obj0;
            }
        };

        /**/

        _STD vector<string> dummyStrList {
            {
                "TRObj::<obj0>::<data2>$0",
                "TRObj::<obj0>::<data2>$1",
                "TRObj::<obj0>::<data2>$2",
                "TRObj::<obj0>::<data2>$3"
            }
        };

        _STD vector<u16> dummyU16List {
            {
                62751ui16,
                7891ui16,
                3152ui16,
                316ui16,
                1135ui16,
                32867ui16
            }
        };

        TRObj writeDummy {
            .data0 = "TRObj::data0",
            .data1 = 3256750268772ui64,
            .data3 = dummyU16List,
            .obj0 = TI1Obj {
                .data0 = 265386243627ui64,
                .obj0 = TI2Obj {
                    .data0 = "TRObj::<obj0>::<obj0>::data0",
                    .data1 = 1234ui32,
                },
                .obj1 = TI2Obj {
                    .data0 = "TRObj::<obj0>::<obj1>::data0",
                    .data1 = 6789ui32
                },
                .data2 = dummyStrList
            }
        };
        {
            auto rootSlot = arch.insertRootSlot();
            writeDummy.serialize(_STD move(rootSlot));
        }

        /**/

        const auto size = archive.size();
        archive.seek(0);

        /**/

        TRObj loadDummy {};
        {
            const auto rootSlot = arch.getRootSlot();
            loadDummy.deserialize(rootSlot);
        }

        /**/

        EXPECT_EQ(loadDummy, writeDummy);
    }

    TEST(StructureArchive, StructUnknownIdentifier) {

        BufferArchive archive {};
        StructuredArchive arch { &archive };

        {
            auto write = arch.insertRootSlot();
            auto slot = write.intoStruct();

            slot.insertRecordSlot("known");
        }

        /**/

        archive.seek(0);

        /**/

        {
            const auto read = arch.getRootSlot();
            auto slot = read.intoStruct();

            EXPECT_FALSE(slot.hasRecordSlot("unknown"));
            EXPECT_TRUE(slot.hasRecordSlot("known"));

            /*
            auto test = slot.getSlot<string>("unknown");
            EXPECT_EQ(test.slot(), nullptr);
             */
        }
    }
}
