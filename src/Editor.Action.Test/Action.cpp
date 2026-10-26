#include "pch.h"

#include <Editor.Action/Action/DelegateAction.hpp>
#include <Editor.Action/Log/ActionLog.hpp>
#include <Engine.Common/Make.hpp>
#include <Engine.Common/Collection/Vector.hpp>
#include <Engine.Pedantic/Clone/Clone.hpp>

using namespace hg;
using namespace hg::editor;

namespace EditorAction {
	namespace {
		[[nodiscard]] Arci<Action> makeAction(int* fwd_ = nullptr, int* bwd_ = nullptr) {
			auto action = Arci<DelegateAction>::create(
				DelegateAction::delegate_type { [fwd_] { if (fwd_) { ++(*fwd_); } } },
				DelegateAction::delegate_type { [bwd_] { if (bwd_) { ++(*bwd_); } } }
			);
			return std::move(action).template into<Action>();
		}
	}

	TEST(DelegateActionTest, StartsPending) {
		auto action = makeAction();

		EXPECT_TRUE(action->isPending());
		EXPECT_FALSE(action->isRunning());
		EXPECT_FALSE(action->isFinished());
	}

	TEST(DelegateActionTest, ApplyRunsForwardAndFinishes) {
		int fwd = 0, bwd = 0;
		auto action = makeAction(&fwd, &bwd);

		EXPECT_TRUE(action->apply().has_value());

		EXPECT_EQ(fwd, 1);
		EXPECT_EQ(bwd, 0);
		EXPECT_TRUE(action->isFinished());
		EXPECT_FALSE(action->isRunning());
		EXPECT_FALSE(action->isPending());
	}

	TEST(DelegateActionTest, UndoRunsBackward) {
		int fwd = 0, bwd = 0;
		auto action = makeAction(&fwd, &bwd);
		ASSERT_TRUE(action->apply().has_value());

		EXPECT_TRUE(action->isReversible());
		EXPECT_TRUE(action->undo().has_value());

		EXPECT_EQ(bwd, 1);
	}

	TEST(DelegateActionTest, RevokeIsUnsupported) {
		auto action = makeAction();

		const auto result = action->revoke();

		ASSERT_FALSE(result.has_value());
		EXPECT_NE(std::string_view { result.error().what() }.size(), 0uLL);
	}

	TEST(DelegateActionTest, WithoutBackwardDelegateIsIrreversible) {
		auto action = Arci<DelegateAction>::create(DelegateAction::delegate_type { [] {} }, DelegateAction::delegate_type {});

		EXPECT_FALSE(action->isReversible());
	}

	TEST(ActionLogTest, UndoOnEmptyLogIsNull) {
		auto log = ActionLog {};

		EXPECT_EQ(log.undo(), nullptr);
		EXPECT_EQ(log.reapply(), nullptr);
	}

	TEST(ActionLogTest, UndoReturnsActionsInReverseOrder) {
		auto log = ActionLog {};
		auto a = makeAction();
		auto b = makeAction();
		log.apply(clone(a));
		log.apply(clone(b));

		EXPECT_EQ(log.undo(), b);
		EXPECT_EQ(log.undo(), a);
		EXPECT_EQ(log.undo(), nullptr);
	}

	TEST(ActionLogTest, FailDropsLastLoggedAction) {
		auto log = ActionLog {};
		auto a = makeAction();
		auto b = makeAction();
		log.apply(clone(a));
		log.apply(clone(b));

		log.fail(clone(b));

		EXPECT_EQ(log.undo(), a);
	}

	TEST(ActionLogTest, LogIsBoundedToMostRecentEntries) {
		auto log = ActionLog {};
		Vector<Arci<Action>> actions {};
		for (u64 i = 0; i < ActionLog::action_log_size + 5uLL; ++i) {
			actions.push_back(makeAction());
			log.apply(clone(actions.back()));
		}

		u64 popped = 0uLL;
		while (log.undo() != nullptr) { ++popped; }

		EXPECT_EQ(popped, ActionLog::action_log_size);
	}

	TEST(ActionLogTest, ReapplyReturnsUndoneAction) {
		auto log = ActionLog {};
		auto a = makeAction();
		log.apply(clone(a));
		ASSERT_EQ(log.undo(), a);

		EXPECT_EQ(log.reapply(), a);
	}

	TEST(ActionLogTest, ReapplyOnEmptyReappliedStackYieldsNothing) {
		auto log = ActionLog {};
		log.apply(makeAction());

		EXPECT_EQ(log.reapply(), nullptr);
	}

	TEST(ActionLogTest, ReapplyReturnsMostRecentlyUndoneFirst) {
		auto log = ActionLog {};
		auto a = makeAction();
		auto b = makeAction();
		log.apply(clone(a));
		log.apply(clone(b));

		// Undo order: b, a. Redo order is the reverse of that: a, b.
		ASSERT_EQ(log.undo(), b);
		ASSERT_EQ(log.undo(), a);

		EXPECT_EQ(log.reapply(), a);
		EXPECT_EQ(log.reapply(), b);
		EXPECT_EQ(log.reapply(), nullptr);
	}

	TEST(ActionLogTest, ReappliedActionCanBeUndoneAgain) {
		auto log = ActionLog {};
		auto a = makeAction();
		log.apply(clone(a));
		ASSERT_EQ(log.undo(), a);
		ASSERT_EQ(log.reapply(), a);

		EXPECT_EQ(log.undo(), a);
	}

	TEST(ActionLogTest, ApplyDiscardsPreviouslyUndoneActions) {
		auto log = ActionLog {};
		log.apply(makeAction());
		ASSERT_NE(log.undo(), nullptr);

		// A fresh action forks the history, so the undone one must not be re-appliable anymore.
		log.apply(makeAction());

		EXPECT_EQ(log.reapply(), nullptr);
	}
}
