#include "pch.h"
/**/

#include <Engine.Common/Memory/MemoryPointer.hpp>

using namespace hg;

namespace Common {
	namespace Memory {
		struct TestPayload01 {
			u64 v0;
			s64 v1;
			std::string v2;
		};

		TEST(MemoryPointer, IntegralTyped) {

			using type = u64;
			using mpt = MemoryPointer<type>;

			mpt mp0 {};
			ASSERT_FALSE(mp0);

			mp0.create();

			ASSERT_TRUE(mp0);
			ASSERT_EQ(*mp0, 0uLL);

			u64 test = 4379689246uLL;
			mp0.store(test);

			ASSERT_EQ(*mp0, test);

			mp0.destroy();
			ASSERT_FALSE(mp0);
		}

		TEST(MemoryPointer, MovingOwner) {

			using type = TestPayload01;
			using mpt = MemoryPointer<type>;

			mpt mp0 {};

			ASSERT_FALSE(mp0);

			mp0.create();

			ASSERT_TRUE(mp0);

			mpt mp1 { std::move(mp0) };

			ASSERT_FALSE(mp0);
			ASSERT_TRUE(mp1);

			mpt mp2 {};

			// Required to prevent optimization
			ASSERT_FALSE(mp2);

			mp2 = std::move(mp1);

			ASSERT_FALSE(mp1);
			ASSERT_TRUE(mp2);

			mp2.destroy();

			ASSERT_FALSE(mp2);
		}

		TEST(MemoryPointer, OwningToNonOwning) {

			using type = TestPayload01;
			using mpt = MemoryPointer<type>;
			using nmpt = mpt::non_owning_type;

			mpt mp0 {};
			nmpt nmp0 {};

			ASSERT_FALSE(mp0);
			ASSERT_FALSE(nmp0);

			mp0.create();
			nmp0 = mp0;

			ASSERT_TRUE(mp0);
			ASSERT_TRUE(nmp0);

			nmpt nmp1 { mp0 };

			ASSERT_TRUE(mp0);
			ASSERT_TRUE(nmp1);

			nmpt nmp2 {};

			// Required to prevent optimization
			ASSERT_FALSE(nmp2);

			nmp2 = nmp0;

			ASSERT_TRUE(nmp0);
			ASSERT_TRUE(nmp2);

			nmp2 = std::move(nmp0);

			ASSERT_FALSE(nmp0);
			ASSERT_TRUE(nmp2);

			mp0.destroy();
		}

		TEST(AtomicMemoryPointer, IntegralTyped) {

			using type = u64;
			using mpt = AtomicMemoryPointer<type>;

			mpt mp0 {};
			ASSERT_FALSE(mp0);

			mp0.create();

			ASSERT_TRUE(mp0);
			ASSERT_EQ(*mp0, 0uLL);

			u64 test = 4379689246uLL;
			mp0.store(test);

			ASSERT_EQ(*mp0, test);

			mp0.destroy();
			ASSERT_FALSE(mp0);
		}

		TEST(AtomicMemoryPointer, MovingOwner) {

			using type = TestPayload01;
			using mpt = AtomicMemoryPointer<type>;

			mpt mp0 {};

			ASSERT_FALSE(mp0);

			mp0.create();

			ASSERT_TRUE(mp0);

			mpt mp1 { std::move(mp0) };

			ASSERT_FALSE(mp0);
			ASSERT_TRUE(mp1);

			mpt mp2 {};

			// Required to prevent optimization
			ASSERT_FALSE(mp2);

			mp2 = std::move(mp1);

			ASSERT_FALSE(mp1);
			ASSERT_TRUE(mp2);

			mp2.destroy();

			ASSERT_FALSE(mp2);
		}

		TEST(AtomicMemoryPointer, OwningToNonOwning) {

			using type = TestPayload01;
			using mpt = AtomicMemoryPointer<type>;
			using nmpt = mpt::non_owning_type;

			mpt mp0 {};
			nmpt nmp0 {};

			ASSERT_FALSE(mp0);
			ASSERT_FALSE(nmp0);

			mp0.create();
			nmp0 = mp0;

			ASSERT_TRUE(mp0);
			ASSERT_TRUE(nmp0);

			nmpt nmp1 { mp0 };

			ASSERT_TRUE(mp0);
			ASSERT_TRUE(nmp1);

			nmpt nmp2 {};

			// Required to prevent optimization
			ASSERT_FALSE(nmp2);

			nmp2 = nmp0;

			ASSERT_TRUE(nmp0);
			ASSERT_TRUE(nmp2);

			nmp2 = std::move(nmp0);

			ASSERT_FALSE(nmp0);
			ASSERT_TRUE(nmp2);

			mp0.destroy();
		}
	}
}
