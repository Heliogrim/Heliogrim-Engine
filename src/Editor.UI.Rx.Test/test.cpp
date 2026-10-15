#include "pch.h"

#include <string>
#include <vector>
#include <Editor.UI.Rx/Subject.hpp>

using namespace hg;
using namespace hg::editor::ui::rx;

namespace EditorUiRx {
	TEST(Subject, PeekReturnsInitialValue) {
		Subject<int> subject { 5 };
		EXPECT_EQ(subject.peek(), 5);
	}

	TEST(Subject, NextStoresAndNotifies) {
		Subject<int> subject { 1 };

		std::vector<int> seen {};
		[[maybe_unused]] auto sub = subject.subscribe([&seen](ref<const int> value_) { seen.push_back(value_); });

		subject.next(2);
		subject.next(3);

		EXPECT_EQ(subject.peek(), 3);
		ASSERT_EQ(seen.size(), 2uLL);
		EXPECT_EQ(seen[0], 2);
		EXPECT_EQ(seen[1], 3);
	}

	TEST(Subject, SubscribeDoesNotReplayCurrentValue) {
		Subject<int> subject { 1 };

		int calls = 0;
		[[maybe_unused]] auto sub = subject.subscribe([&calls](ref<const int>) { ++calls; });

		EXPECT_EQ(calls, 0);
	}

	TEST(Subject, MultipleSubscribersAllNotified) {
		Subject<int> subject { 0 };

		int a = 0, b = 0;
		[[maybe_unused]] auto sa = subject.subscribe([&a](ref<const int> v_) { a = v_; });
		[[maybe_unused]] auto sb = subject.subscribe([&b](ref<const int> v_) { b = v_; });

		subject.next(7);

		EXPECT_EQ(a, 7);
		EXPECT_EQ(b, 7);
	}

	TEST(Subject, SubscriptionIdsAreSequential) {
		Subject<int> subject { 0 };

		auto s0 = subject.subscribe([](ref<const int>) {});
		auto s1 = subject.subscribe([](ref<const int>) {});

		EXPECT_EQ(s0.id.value, 0u);
		EXPECT_EQ(s1.id.value, 1u);
	}

	TEST(Subject, UnsubscribeStopsNotifications) {
		Subject<int> subject { 0 };

		int a = 0, b = 0;
		auto sa = subject.subscribe([&a](ref<const int> v_) { a = v_; });
		[[maybe_unused]] auto sb = subject.subscribe([&b](ref<const int> v_) { b = v_; });

		std::move(sa).unsubscribe();
		subject.next(9);

		EXPECT_EQ(a, 0);
		EXPECT_EQ(b, 9);
	}

	TEST(Subject, UnsubscribeUnknownIdIsNoop) {
		Subject<int> subject { 0 };
		EXPECT_NO_THROW(subject.unsubscribe(Id { 42u }));
	}

	TEST(Subject, IdsDoNotCollideAfterUnsubscribingMiddle) {
		Subject<int> subject { 0 };

		auto s0 = subject.subscribe([](ref<const int>) {});
		auto s1 = subject.subscribe([](ref<const int>) {});
		std::move(s0).unsubscribe();

		// New id derives from the highest remaining id, so the live subscriber must survive.
		int calls = 0;
		auto s2 = subject.subscribe([&calls](ref<const int>) { ++calls; });
		EXPECT_NE(s2.id.value, s1.id.value);

		subject.next(1);
		EXPECT_EQ(calls, 1);
	}


	TEST(Subject, StringPayload) {
		Subject<std::string> subject { "a" };

		std::string seen {};
		[[maybe_unused]] auto sub = subject.subscribe([&seen](ref<const std::string> v_) { seen = v_; });
		subject.next("b");

		EXPECT_EQ(seen, "b");
		EXPECT_EQ(subject.peek(), "b");
	}
}
