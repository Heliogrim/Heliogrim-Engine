#include "pch.h"

#include <Engine.Common/String.hpp>
#include <Engine.Common/Variant.hpp>
#include <Engine.Common/Collection/InlineAutoArray.hpp>
#include <Engine.Common/Collection/ProxyVector.hpp>
#include <Engine.Common/Concurrent/SharedMemoryReference.hpp>
#include <Engine.Common/Managed/Rc.hpp>
#include <Engine.Common/Math/Transformation.hpp>
#include <Engine.Common/Math/Vector.hpp>
#include <Engine.Common/Memory/MemoryPointer.hpp>

using namespace hg;

TEST(__DummyTest__, Exists) {
	EXPECT_TRUE(true);
}

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

	namespace Managed {
		TEST(RefCounted, Create) {

			struct Payload {};
			auto obj = Rc<Payload>::create();
			auto cpy = obj;
			auto mvt = std::move(obj);

			EXPECT_EQ(mvt.ref_count(), 2);
		}

		TEST(RefCounted, Release) {

			struct Payload {};
			auto obj = Rc<Payload>::create();
			auto cpy = obj;

			EXPECT_EQ(obj.ref_count(), 2);
			EXPECT_EQ(obj.release().get(), nullptr);

			cpy.reset();
			EXPECT_EQ(obj.ref_count(), 1);

			auto val = obj.release();
			EXPECT_NE(val.get(), nullptr);
			val.destroy();

			EXPECT_EQ(obj.ref_count(), 0);
		}

		TEST(RefCountedIntrusive, Create) {

			struct Payload : public RcFromThis<Payload> {};

			auto obj = Rci<Payload>::create();
			auto cpy = obj;
			auto mvt = std::move(obj);

			EXPECT_EQ(mvt.ref_count(), 2);
		}

		TEST(RefCountedIntrusive, Release) {

			struct Payload : public RcFromThis<Payload> {};

			auto obj = Rci<Payload>::create();
			auto cpy = obj;

			EXPECT_EQ(obj.ref_count(), 2);
			EXPECT_EQ(obj.release().get(), nullptr);

			cpy.reset();
			EXPECT_EQ(obj.ref_count(), 1);

			auto val = obj.release();
			EXPECT_NE(val.get(), nullptr);
			val.destroy();

			EXPECT_EQ(obj.ref_count(), 0);
		}

		TEST(RefCountedIntrusive, ReplicatedIntrusive) {

			struct Payload : public RcFromThis<Payload> {};

			auto obj = Rci<Payload>::create();
			auto cpy = obj;

			EXPECT_EQ(obj.ref_count(), 2);

			auto self_copy = obj->rci_from_this();

			EXPECT_EQ(obj.ref_count(), 3);

			auto leveled_self_copy = self_copy->rci_from_this();

			EXPECT_EQ(obj.ref_count(), 4);

			cpy.reset();
			self_copy.reset();

			EXPECT_EQ(obj.ref_count(), 2);
		}

		TEST(AtomicRefCounted, Create) {

			struct Payload {};
			//auto obj = Arc::create<Payload>();
		}

		TEST(AtomicRefCountedIntrusive, Create) {

			struct Payload : public ArcFromThis<Payload> {};
			auto obj = Arci<Payload>::create();
		}

		TEST(AtomicRefCountedIntrusive, ReplicatedIntrusive) {

			struct Payload : public ArcFromThis<Payload> {};

			auto obj = Arci<Payload>::create();
			auto cpy = obj;

			EXPECT_EQ(obj.ref_count(), 2);

			auto self_copy = obj->arci_from_this<>();

			EXPECT_EQ(obj.ref_count(), 3);

			auto leveled_self_copy = self_copy->arci_from_this();

			EXPECT_EQ(obj.ref_count(), 4);
		}
	}

	namespace Collection {
		TEST(ProxyVector, IntegralTyped) {
			//
			using type = u64;

			ProxyVector<>::proxy_holder_type proxy {
				sizeof(type),
				nullptr,
				[](ptr<void> this_, ptr<void> other_) {
					new(this_) type(*static_cast<ptr<type>>(other_));
				},
				[](ptr<void> this_, ptr<void> other_) noexcept {
					new(this_) type(std::move(*static_cast<ptr<type>>(other_)));
				},
				[](ptr<void> this_, ptr<void> other_) noexcept {
					*static_cast<ptr<type>>(this_) = *static_cast<ptr<type>>(other_);
				},
				[](ptr<void> this_, ptr<void> other_) noexcept {
					*static_cast<ptr<type>>(this_) = std::move(*static_cast<ptr<type>>(other_));
				},
			};

			auto vector = ProxyVector<>::make(proxy);

			vector.emplace_back<type>({});
		}

		TEST(ProxyVector, SmartPointerTyped) {
			//
			using type = std::shared_ptr<u64>;

			ProxyVector<>::proxy_holder_type proxy {
				sizeof(type),
				[](ptr<void> this_) noexcept {
					static_cast<ptr<type>>(this_)->~shared_ptr();
				},
				[](ptr<void> this_, ptr<void> other_) {
					new(this_) type(*static_cast<ptr<type>>(other_));
				},
				[](ptr<void> this_, ptr<void> other_) noexcept {
					new(this_) type(std::move(*static_cast<ptr<type>>(other_)));
				},
				[](ptr<void> this_, ptr<void> other_) noexcept {
					static_cast<ptr<type>>(this_)->operator=(*static_cast<ptr<type>>(other_));
				},
				[](ptr<void> this_, ptr<void> other_) noexcept {
					static_cast<ptr<type>>(this_)->operator=(std::move(*static_cast<ptr<type>>(other_)));
				},
			};

			auto vector = ProxyVector<>::make(proxy);

			vector.emplace_back<type>({});
		}

		TEST(ProxyVector, CustomTyped) {
			//
			using type = class TestPayload {
			private:
				char _payload[127] = {};
			};

			ProxyVector<>::proxy_holder_type proxy {
				sizeof(type),
				[](ptr<void> this_) noexcept {
					static_cast<ptr<type>>(this_)->~TestPayload();
				},
				[](ptr<void> this_, ptr<void> other_) {
					new(this_) type(*static_cast<ptr<type>>(other_));
				},
				[](ptr<void> this_, ptr<void> other_) noexcept {
					new(this_) type(std::move(*static_cast<ptr<type>>(other_)));
				},
				[](ptr<void> this_, ptr<void> other_) noexcept {
					static_cast<ptr<type>>(this_)->operator=(*static_cast<ptr<type>>(other_));
				},
				[](ptr<void> this_, ptr<void> other_) noexcept {
					static_cast<ptr<type>>(this_)->operator=(std::move(*static_cast<ptr<type>>(other_)));
				}
			};

			auto vector = ProxyVector<>::make(proxy);

			vector.emplace_back<type>({});
		}

		TEST(ProxyVector, Behaviour) {
			//
			using type = u64;

			constexpr ProxyVector<>::proxy_holder_type proxy {
				sizeof(type),
				nullptr,
				[](ptr<void> this_, ptr<void> other_) {
					new(this_) type(*static_cast<ptr<type>>(other_));
				},
				[](ptr<void> this_, ptr<void> other_) noexcept {
					new(this_) type(std::move(*static_cast<ptr<type>>(other_)));
				},
				[](ptr<void> this_, ptr<void> other_) noexcept {
					*static_cast<ptr<type>>(this_) = *static_cast<ptr<type>>(other_);
				},
				[](ptr<void> this_, ptr<void> other_) noexcept {
					*static_cast<ptr<type>>(this_) = std::move(*static_cast<ptr<type>>(other_));
				},
			};

			auto vector = ProxyVector<>::make(proxy);

			//
			EXPECT_TRUE(vector.empty());
			EXPECT_EQ(vector.size(), 0);
			EXPECT_EQ(vector.size<type>(), 0);

			//
			vector.emplace_back<type>(32);

			EXPECT_FALSE(vector.empty());
			EXPECT_EQ(vector.size(), 1);
			EXPECT_EQ(vector.size<type>(), 1);
			EXPECT_GE(vector.capacity(), 1);
			EXPECT_GE(vector.capacity<type>(), 1);

			//
			for (auto c = 0; c < 23; ++c) {
				vector.emplace_back<type>(32 - c);
			}

			EXPECT_FALSE(vector.empty());
			EXPECT_EQ(vector.size(), 24);
			EXPECT_EQ(vector.size<type>(), 24);
			EXPECT_GE(vector.capacity(), 24);
			EXPECT_GE(vector.capacity<type>(), 24);

			//
			auto counter = 0;
			for (auto& entry : vector.typed<type>()) {
				++counter;
			}

			EXPECT_EQ(counter, 24);
		}

		TEST(ProxyVector, ConstructWithValueList) {
			//
			using type = string;

			ProxyVector<>::proxy_holder_type proxy {
				sizeof(type),
				[](ptr<void> this_) noexcept {
					static_cast<ptr<type>>(this_)->~basic_string();
				},
				[](ptr<void> this_, ptr<void> other_) {
					new(this_) type(*static_cast<ptr<type>>(other_));
				},
				[](ptr<void> this_, ptr<void> other_) noexcept {
					new(this_) type(std::move(*static_cast<ptr<type>>(other_)));
				},
				[](ptr<void> this_, ptr<void> other_) noexcept {
					static_cast<ptr<type>>(this_)->operator=(*static_cast<ptr<type>>(other_));
				},
				[](ptr<void> this_, ptr<void> other_) noexcept {
					static_cast<ptr<type>>(this_)->operator=(std::move(*static_cast<ptr<type>>(other_)));
				},
			};

			auto vector = ProxyVector<>::make<type>(
				proxy,
				string { "String 01" },
				string { "String 02" },
				string { "String 03" },
				string { "String 04" }
			);
		}

		TEST(ProxyVector, Copying) {
			//
			using type = u64;

			constexpr ProxyVector<>::proxy_holder_type proxy {
				sizeof(type),
				nullptr,
				[](ptr<void> this_, ptr<void> other_) {
					new(this_) type(*static_cast<ptr<type>>(other_));
				},
				[](ptr<void> this_, ptr<void> other_) noexcept {
					new(this_) type(std::move(*static_cast<ptr<type>>(other_)));
				},
				[](ptr<void> this_, ptr<void> other_) noexcept {
					*static_cast<ptr<type>>(this_) = *static_cast<ptr<type>>(other_);
				},
				[](ptr<void> this_, ptr<void> other_) noexcept {
					*static_cast<ptr<type>>(this_) = std::move(*static_cast<ptr<type>>(other_));
				},
			};

			auto srcVec0 = ProxyVector<>::make<type>(proxy, 1, 2, 3, 4);
			auto srcVec1 = ProxyVector<>::make<type>(proxy, 5, 6, 7);

			EXPECT_EQ(srcVec0.size(), 4);
			EXPECT_EQ(srcVec1.size(), 3);

			// Copy Construct
			auto tv0 { srcVec0 };

			EXPECT_FALSE(tv0.empty());
			EXPECT_EQ(tv0.size(), 4);
			EXPECT_EQ(srcVec0.size(), tv0.size());

			EXPECT_EQ(tv0.operator[]<type>(0), 1);
			EXPECT_EQ(tv0.operator[]<type>(1), 2);
			EXPECT_EQ(tv0.operator[]<type>(2), 3);
			EXPECT_EQ(tv0.operator[]<type>(3), 4);

			// Copy Assign - Lower Size
			tv0 = srcVec1;

			EXPECT_FALSE(tv0.empty());
			EXPECT_EQ(tv0.size(), 4);
			EXPECT_EQ(srcVec0.size(), tv0.size());
			EXPECT_EQ(srcVec1.size(), 3);

			EXPECT_EQ(tv0.operator[]<type>(0), 5);
			EXPECT_EQ(tv0.operator[]<type>(1), 6);
			EXPECT_EQ(tv0.operator[]<type>(2), 7);
			EXPECT_EQ(tv0.operator[]<type>(3), 4);

			// Copy Construct
			auto tv1 { srcVec1 };

			EXPECT_FALSE(tv1.empty());
			EXPECT_EQ(tv1.size(), 3);
			EXPECT_EQ(srcVec1.size(), tv1.size());

			EXPECT_EQ(tv1.operator[]<type>(0), 5);
			EXPECT_EQ(tv1.operator[]<type>(1), 6);
			EXPECT_EQ(tv1.operator[]<type>(2), 7);

			// Copy Assign - Bigger Size
			tv1 = srcVec0;

			EXPECT_FALSE(tv1.empty());
			EXPECT_EQ(tv1.size(), 4);
			EXPECT_EQ(srcVec0.size(), tv1.size());
			EXPECT_EQ(srcVec1.size(), 3);

			EXPECT_EQ(tv1.operator[]<type>(0), 1);
			EXPECT_EQ(tv1.operator[]<type>(1), 2);
			EXPECT_EQ(tv1.operator[]<type>(2), 3);
			EXPECT_EQ(tv1.operator[]<type>(3), 4);
		}

		TEST(ProxyVector, Moveing) {
			//
			using type = u64;

			constexpr ProxyVector<>::proxy_holder_type proxy {
				sizeof(type),
				nullptr,
				[](ptr<void> this_, ptr<void> other_) {
					new(this_) type(*static_cast<ptr<type>>(other_));
				},
				[](ptr<void> this_, ptr<void> other_) noexcept {
					new(this_) type(std::move(*static_cast<ptr<type>>(other_)));
				},
				[](ptr<void> this_, ptr<void> other_) noexcept {
					*static_cast<ptr<type>>(this_) = *static_cast<ptr<type>>(other_);
				},
				[](ptr<void> this_, ptr<void> other_) noexcept {
					*static_cast<ptr<type>>(this_) = std::move(*static_cast<ptr<type>>(other_));
				},
			};

			auto srcVec0 = ProxyVector<>::make<type>(proxy, 1, 2, 3, 4);
			auto srcVec1 = ProxyVector<>::make<type>(proxy, 5, 6, 7);

			EXPECT_EQ(srcVec0.size(), 4);
			EXPECT_EQ(srcVec1.size(), 3);

			// Move Construct
			auto tv0 { std::move(srcVec0) };

			EXPECT_FALSE(tv0.empty());
			EXPECT_EQ(tv0.size(), 4);
			EXPECT_TRUE(srcVec0.empty());
			EXPECT_EQ(srcVec0.size(), 0);

			EXPECT_EQ(tv0.operator[]<type>(0), 1);
			EXPECT_EQ(tv0.operator[]<type>(1), 2);
			EXPECT_EQ(tv0.operator[]<type>(2), 3);
			EXPECT_EQ(tv0.operator[]<type>(3), 4);

			// Move Assign ( internal swap )
			tv0 = std::move(srcVec1);

			EXPECT_FALSE(tv0.empty());
			EXPECT_EQ(tv0.size(), 3);
			EXPECT_FALSE(srcVec1.empty());
			EXPECT_EQ(srcVec1.size(), 4);

			EXPECT_EQ(tv0.operator[]<type>(0), 5);
			EXPECT_EQ(tv0.operator[]<type>(1), 6);
			EXPECT_EQ(tv0.operator[]<type>(2), 7);
		}

		TEST(ProxyVector, Erase) {
			//
			using type = u64;

			constexpr ProxyVector<>::proxy_holder_type proxy {
				sizeof(type),
				nullptr,
				[](ptr<void> this_, ptr<void> other_) {
					new(this_) type(*static_cast<ptr<type>>(other_));
				},
				[](ptr<void> this_, ptr<void> other_) noexcept {
					new(this_) type(std::move(*static_cast<ptr<type>>(other_)));
				},
				[](ptr<void> this_, ptr<void> other_) noexcept {
					*static_cast<ptr<type>>(this_) = *static_cast<ptr<type>>(other_);
				},
				[](ptr<void> this_, ptr<void> other_) noexcept {
					*static_cast<ptr<type>>(this_) = std::move(*static_cast<ptr<type>>(other_));
				},
			};

			//
			{
				auto vector = ProxyVector<>::make<type>(proxy, 1, 2, 3, 4, 5, 6, 7, 8, 9);
				EXPECT_EQ(vector.size(), 9);

				auto where = vector.typed<type>().begin();
				where += 3;

				vector.erase<type>(where);

				EXPECT_EQ(vector.size(), 8);

				EXPECT_EQ(vector.at<type>(0), 1);
				EXPECT_EQ(vector.at<type>(1), 2);
				EXPECT_EQ(vector.at<type>(2), 3);
				// ProxyVector<Stable_ = false> -> remove-erase-idiom
				EXPECT_EQ(vector.at<type>(3), 9);
				EXPECT_EQ(vector.at<type>(4), 5);
				EXPECT_EQ(vector.at<type>(5), 6);
				EXPECT_EQ(vector.at<type>(6), 7);
				EXPECT_EQ(vector.at<type>(7), 8);
			}

			//
			{
				auto vector = ProxyVector<>::make<type>(proxy, 1, 2, 3, 4, 5, 6, 7, 8, 9);
				EXPECT_EQ(vector.size(), 9);

				auto fst = vector.typed<type>().begin() + 2;
				auto lst = vector.typed<type>().begin() + 5;

				vector.erase<type>(fst, lst);

				EXPECT_EQ(vector.size(), 6);

				EXPECT_EQ(vector.at<type>(0), 1);
				EXPECT_EQ(vector.at<type>(1), 2);
				EXPECT_EQ(vector.at<type>(2), 6);
				EXPECT_EQ(vector.at<type>(3), 7);
				EXPECT_EQ(vector.at<type>(4), 8);
				EXPECT_EQ(vector.at<type>(5), 9);
			}

			//
			{
				auto vector = ProxyVector<>::make<type>(proxy, 1, 2, 3, 4, 5, 6, 7, 8, 9);
				EXPECT_EQ(vector.size(), 9);

				auto where = vector.typed<type>().begin() + 7;

				vector.erase(where.operator->());

				EXPECT_EQ(vector.size(), 8);

				EXPECT_EQ(vector.at<type>(0), 1);
				EXPECT_EQ(vector.at<type>(1), 2);
				EXPECT_EQ(vector.at<type>(2), 3);
				EXPECT_EQ(vector.at<type>(3), 4);
				EXPECT_EQ(vector.at<type>(4), 5);
				EXPECT_EQ(vector.at<type>(5), 6);
				EXPECT_EQ(vector.at<type>(6), 7);
				EXPECT_EQ(vector.at<type>(7), 9);
			}

			//
			{
				auto vector = ProxyVector<>::make<type>(proxy, 1, 2, 3, 4, 5, 6, 7, 8, 9);
				EXPECT_EQ(vector.size(), 9);

				auto fst = vector.typed<type>().begin() + 4;
				auto lst = vector.typed<type>().begin() + 8;
				// [4,5,6,7,8)
				// [1,2,3,4, >> 5,6,7,8, << 9]
				// [1,2,3,4,9 ...]

				vector.erase(fst.operator->(), lst.operator->());

				EXPECT_EQ(vector.size(), 5);

				EXPECT_EQ(vector.at<type>(0), 1);
				EXPECT_EQ(vector.at<type>(1), 2);
				EXPECT_EQ(vector.at<type>(2), 3);
				EXPECT_EQ(vector.at<type>(3), 4);
				EXPECT_EQ(vector.at<type>(4), 9);
			}
		}
	}

	namespace Collection {
		TEST(InlineAutoArray, ZeroSize) {
			//InlineAutoArray<size_t, 0> tmp {};
			// Result: Will not compile due to min capacity constraint
		}

		TEST(InlineAutoArray, SingleSize) {
			InlineAutoArray<size_t, 1> tmp {};

			EXPECT_EQ(tmp.size(), 0);
			tmp.emplace_back(0);
			EXPECT_EQ(tmp.size(), 1);
		}

		TEST(InlineAutoArray, InlineSize) {
			InlineAutoArray<size_t, 4> tmp {};

			EXPECT_EQ(tmp.size(), 0);
			tmp.emplace_back(0);
			tmp.emplace_back(1);
			tmp.emplace_back(2);
			EXPECT_EQ(tmp.size(), 3);
		}

		TEST(InlineAutoArray, OverflowSize) {
			InlineAutoArray<size_t, 4> tmp {};

			EXPECT_EQ(tmp.size(), 0);

			tmp.emplace_back(0);
			tmp.emplace_back(1);
			tmp.emplace_back(2);
			tmp.emplace_back(3);
			tmp.emplace_back(4);
			tmp.emplace_back(5);

			EXPECT_EQ(tmp.size(), 6);
		}

		TEST(InlineAutoArray, PopInlineBack) {

			InlineAutoArray<size_t, 4> tmp {};

			tmp.emplace_back(0);
			tmp.emplace_back(1);

			EXPECT_NE(*tmp.begin(), *(--tmp.end()));

			tmp.pop_back();

			EXPECT_EQ(tmp.size(), 1);
			EXPECT_EQ(*tmp.begin(), *(--tmp.end()));
		}

		TEST(InlineAutoArray, PopExternalBack) {

			InlineAutoArray<size_t, 1> tmp {};

			tmp.emplace_back(0);
			tmp.emplace_back(1);
			tmp.emplace_back(2);

			EXPECT_NE(*tmp.begin(), *(--tmp.end()));

			tmp.pop_back();

			EXPECT_EQ(tmp.size(), 2);
			EXPECT_NE(*tmp.begin(), *(--tmp.end()));
			EXPECT_EQ(*(++tmp.begin()), *(--tmp.end()));
		}

		TEST(InlineAutoArray, PopMixedBack) {

			InlineAutoArray<size_t, 2> tmp {};

			tmp.emplace_back(0);
			tmp.emplace_back(1);
			tmp.emplace_back(20);
			tmp.emplace_back(21);

			EXPECT_NE(*tmp.begin(), *(--tmp.end()));

			tmp.pop_back();
			tmp.pop_back();

			tmp.pop_back();
			tmp.pop_back();

			EXPECT_EQ(tmp.size(), 0);
		}

		TEST(InlineAutoArray, Iterate) {

			InlineAutoArray<size_t, 8> tmp {};

			for (size_t i = 0; i < 0xF; ++i) {
				tmp.emplace_back(i);
			}

			EXPECT_EQ(tmp.size(), 0xF);

			size_t c { 0 };
			for (const auto& entry : tmp) {
				EXPECT_EQ(entry, c++);
			}

			EXPECT_EQ(c, 0xF);
		}

		TEST(InlineAutoArray, Nested) {

			using inner_type = InlineAutoArray<smr<size_t>, 1>;
			using outer_type = InlineAutoArray<inner_type, 1>;

			outer_type tmp {};
			tmp.emplace_back(inner_type {});
		}
	}

	TEST(Variant, Default) {
		auto obj = Variant<std::monostate> {};
		EXPECT_FALSE(obj.valueless_by_exception());
	}

	TEST(Variant, IsTypeCheck) {
		auto obj = Variant<u8, u16, u32, String, StringView> {};

		obj = u8 {};
		EXPECT_TRUE(obj.is<u8>());
		EXPECT_FALSE(obj.is<u16>());
		EXPECT_FALSE(obj.is<StringView>());

		obj = u32 {};
		EXPECT_FALSE(obj.is<u8>());
		EXPECT_TRUE(obj.is<u32>());
		EXPECT_FALSE(obj.is<StringView>());

		obj = "View"sv;
		EXPECT_FALSE(obj.is<String>());
		EXPECT_TRUE(obj.is<StringView>());
	}

	TEST(Variant, AsType) {
		auto obj = Variant<u32, StringView> {};

		obj = "Test View"sv;
		EXPECT_EQ(obj.as<StringView>(), "Test View"sv);

		obj = u32 { 675714 };
		EXPECT_EQ(obj.as<u32>(), 675714);
	}

	struct VariantTestBase {
		[[nodiscard]] int val() const noexcept {
			return 45;
		}

		[[nodiscard]] int val() noexcept {
			return 86;
		}
	};

	struct VariantTestDerived1 : public VariantTestBase {};

	struct VariantTestDerived2 : public VariantTestBase {};

	struct VariantTestDerivedLayer : public VariantTestDerived2 {};

	TEST(Variant, CommonTypeInvocation) {
		using type = Variant<VariantTestBase, VariantTestDerived1, VariantTestDerived2, VariantTestDerivedLayer>;

		auto obj = type {};
		EXPECT_EQ(obj->val(), 86);

		const auto cobj = type {};
		EXPECT_EQ(cobj->val(), 45);
	}

	TEST(Variant, InvokeFromShared) {
		auto obj = Variant<VariantTestDerived1, VariantTestDerived2, VariantTestDerivedLayer> {};
		EXPECT_EQ(obj.apply([](auto&& stored_) { return std::forward<decltype(stored_)>(stored_).val(); }), 86);

		const auto cobj = Variant<VariantTestDerived1, VariantTestDerived2, VariantTestDerivedLayer> {};
		EXPECT_EQ(cobj.apply([](const auto& stored_) { return stored_.val(); }), 45);
	}
}

TEST(MathVectorTest, vectorInit2d) {
	using namespace hg::math;
	const int x = 10;
	const int y = 35;

	const vec2_t<int> test = vec2_t<int>(x, y);

	EXPECT_EQ(x, test.x);
	EXPECT_EQ(y, test.y);
}

TEST(MathVectorTest, vectorDownCast) {
	using namespace hg::math;
	const vec3_t<int> src = vec3_t<int>(30, 40, 50);
	const vec2_t<int> dst = vec2_t<int>(src);

	EXPECT_EQ(src.x, dst.x);
	EXPECT_EQ(src.y, dst.y);
}

TEST(MathVectorTest, vectorUpCast) {
	using namespace hg::math;
	const vec3_t<int> src = vec3_t<int>(70, 90, 110);
	const vec4_t<int> dst = vec4_t<int>(src, 5);

	EXPECT_EQ(src.x, dst.x);
	EXPECT_EQ(src.y, dst.y);
	EXPECT_EQ(src.z, dst.z);
}

TEST(MathVectorTest, vectorNormalize) {
	using namespace hg::math;
	vec3_t<float> v = vec3_t<float>(5.F);
	v.normalize();

	EXPECT_FLOAT_EQ(v.x, 1.F / sqrt(3.F));
	EXPECT_FLOAT_EQ(v.y, 1.F / sqrt(3.F));
	EXPECT_FLOAT_EQ(v.z, 1.F / sqrt(3.F));
}

TEST(Transform, initZero) {
	using namespace hg::math;
	Transformation t = ZeroTransformation();

	EXPECT_EQ(t.position(), vec3());
	EXPECT_EQ(t.rotation(), vec3());
	EXPECT_EQ(t.scale(), vec3(1.F));
}

TEST(Quaternion, fromToEuler) {
	using namespace hg::math;
	const float r45f = glm::radians(45.F);
	const float r0f = glm::radians(0.F);

	quaternion q = quaternion::euler(
		{
			r45f,
			r0f,
			r0f
		}
	);

	const vec3 ve = q.euler();
	EXPECT_FLOAT_EQ(ve.x, r45f);
	EXPECT_FLOAT_EQ(ve.y, r0f);
	EXPECT_FLOAT_EQ(ve.z, r0f);
}
