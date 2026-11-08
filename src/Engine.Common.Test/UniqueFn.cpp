#include "pch.h"

#include <memory>
#include <type_traits>
#include <Engine.Common/Functional/UniqueFn.hpp>

using namespace hg;

namespace Common {
	namespace {
		struct Big {
			char data[128];

			void operator()() const noexcept {}
		};

		struct Guard {
			int* counter;

			explicit Guard(int* const counter_) :
				counter(counter_) {}

			Guard(Guard&& other_) noexcept :
				counter(std::exchange(other_.counter, nullptr)) {}

			~Guard() {
				if (counter) {
					++(*counter);
				}
			}

			void operator()() const noexcept {}
		};
	}

	TEST(UniqueFn, InvokesMoveOnlyCapture) {
		auto value = std::make_unique<int>(42);
		UniqueFn<int()> fn { [v = std::move(value)]() { return *v; } };

		ASSERT_TRUE(static_cast<bool>(fn));
		EXPECT_EQ(std::move(fn)(), 42);
		EXPECT_FALSE(static_cast<bool>(fn));
	}

	TEST(UniqueFn, MoveTransfersOwnership) {
		UniqueFn<int(int)> from { [](const int v_) { return v_ + 1; } };
		UniqueFn<int(int)> to { std::move(from) };

		EXPECT_FALSE(static_cast<bool>(from));
		ASSERT_TRUE(static_cast<bool>(to));
		EXPECT_EQ(std::move(to)(1), 2);
	}

	TEST(UniqueFn, DestroysCapture) {
		int destroyed = 0; {
			UniqueFn<void()> fn { Guard { &destroyed } };
			EXPECT_EQ(destroyed, 0);
		}
		EXPECT_EQ(destroyed, 1);
	}

	TEST(UniqueFn, RejectsOversizedCapture) {
		static_assert(!std::is_constructible_v<UniqueFn<void()>, Big>);
		static_assert(std::is_constructible_v<UniqueFn<void(), 256>, Big>);
		SUCCEED();
	}
}
