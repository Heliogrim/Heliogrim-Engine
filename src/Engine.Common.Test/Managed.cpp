#include "pch.h"
/**/

#include <Engine.Common/String.hpp>
#include <Engine.Common/UniqueValue.hpp>
#include <Engine.Common/Concurrent/SharedMemoryReference.hpp>
#include <Engine.Common/Managed/Rc.hpp>

using namespace hg;

namespace Common {
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

	/**/

	TEST(UniqueValue, TrivialDefault) {
		using type = UniqueValue<size_t>;
		auto obj = type {};
		EXPECT_TRUE(obj.empty());
	}

	TEST(UniqueValue, TrivialConstruct) {
		using type = UniqueValue<size_t>;
		auto obj = type { std::in_place };
		EXPECT_FALSE(obj.empty());
	}

	TEST(UniqueValue, TrivialValueConstruct) {
		using type = UniqueValue<size_t>;
		auto obj = type { 657776296707uLL };
		EXPECT_FALSE(obj.empty());
		EXPECT_EQ(obj.get(), 657776296707uLL);
	}

	TEST(UniqueValue, StringDefault) {
		using type = UniqueValue<String>;
		auto obj = type {};
		EXPECT_TRUE(obj.empty());
	}

	TEST(UniqueValue, StringConstruct) {
		using type = UniqueValue<String>;
		auto obj = type { std::in_place };
		EXPECT_FALSE(obj.empty());
	}

	TEST(UniqueValue, StringValueConstruct) {
		using type = UniqueValue<String>;
		auto obj = type { "Hello World" };
		EXPECT_FALSE(obj.empty());
		EXPECT_EQ(obj.get(), "Hello World");
	}

	TEST(UniqueValue, DestroyerDefault) {

		bool ack = false;

		struct Destroyer {
			ptr<bool> ackRef = nullptr;

			void operator()([[maybe_unused]] const ptr<std::monostate> val_) const noexcept {
				ASSERT_NE(ackRef, nullptr);
				EXPECT_FALSE(*ackRef);
				*ackRef = true;
			}
		};

		/**/

		{
			using type = UniqueValue<std::monostate, Destroyer>;
			auto dest = Destroyer { std::addressof(ack) };
			auto obj = type { std::move(dest) };

			EXPECT_FALSE(obj.empty());
		}

		/**/

		EXPECT_TRUE(ack);
	}

	TEST(UniqueValue, ComplexMove) {

		size_t invocationCount = 0uLL;

		struct Destroyer {
			ptr<decltype(invocationCount)> _invCountRef;

			void operator()(const ptr<String> val_) const noexcept {
				EXPECT_EQ(*val_, "Hello World");
				std::ignore = std::move(*val_);
				++(*_invCountRef);
			}
		};

		/**/

		{
			using type = UniqueValue<String, Destroyer>;

			auto dest = Destroyer { std::addressof(invocationCount) };
			auto obj0 = type { std::move(dest), "Hello World" };
			auto obj1 = type {};

			EXPECT_FALSE(obj0.empty());
			EXPECT_TRUE(obj1.empty());

			obj1 = std::move(obj0);

			EXPECT_TRUE(obj0.empty());
			EXPECT_FALSE(obj1.empty());
		}

		/**/

		EXPECT_EQ(invocationCount, 1uLL);

	}
}
