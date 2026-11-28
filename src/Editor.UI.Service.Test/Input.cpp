#include "pch.h"

/**/

import Editor.Action;
import Editor.UI.Service;
import Engine.Common;
import Engine.Diagnostics;

using namespace hg;
using namespace hg::editor::ui::service;

namespace EditorService {
	namespace {
		[[nodiscard]] InlineAutoArray<u32, 4> keys(std::initializer_list<u32> keys_) {
			InlineAutoArray<u32, 4> out {};
			for (const auto key : keys_) { out.emplace_back(key); }
			return out;
		}

		[[nodiscard]] InputBinding binding(
			std::initializer_list<u32> keys_,
			int* counter_ = nullptr,
			std::function<bool()> condition_ = nullptr,
			Opt<ref<const engine::diag::DiagnosticCodeInfo>> error_ = None
		) {
			return InputBinding {
				.label = "test",
				.binding = keys(keys_),
				.condition = std::move(condition_),
				.invoke = [counter_, error_ = std::move(error_)]() -> Result<Arci<editor::Action>, engine::diag::Diagnostic> {
					if (counter_ != nullptr) { ++(*counter_); }
					if (error_ != None) { return engine::diag::Diagnostic::error(*error_, "failed"); }
					return Expected { Arci<editor::DelegateAction>::create().into<editor::Action>() };
				}
			};
		}
	}

	TEST(InputBindingListTest, InvokeWithoutBindingsIsNone) {
		const InputBindingList list {};
		EXPECT_FALSE(list.invoke(keys({ 1u })).has_value());
	}

	TEST(InputBindingListTest, AddRejectsIdenticalKeyChord) {
		InputBindingList list {};

		EXPECT_TRUE(list.addBinding(binding({ 1u, 2u })));
		EXPECT_FALSE(list.addBinding(binding({ 1u, 2u })));
		EXPECT_TRUE(list.addBinding(binding({ 2u, 1u })));
		EXPECT_TRUE(list.addBinding(binding({ 1u })));
	}

	TEST(InputBindingListTest, InvokeRunsMatchingBindingOnly) {
		InputBindingList list {};
		int a = 0, b = 0;
		ASSERT_TRUE(list.addBinding(binding({ 1u }, &a)));
		ASSERT_TRUE(list.addBinding(binding({ 2u }, &b)));

		EXPECT_TRUE(list.invoke(keys({ 2u })).has_value());

		EXPECT_EQ(a, 0);
		EXPECT_EQ(b, 1);
	}

	TEST(InputBindingListTest, FailingBindingReturnsTheError) {
		InputBindingList list {};
		int a = 0;
		ASSERT_TRUE(list.addBinding(binding({ 1u }, &a, nullptr, SomeRef(editor::codes::edt("no-root-level")))));

		const auto result = list.invoke(keys({ 1u }));

		ASSERT_TRUE(result.has_value());
		ASSERT_FALSE(result->has_value());
		EXPECT_EQ(result->error().code(), &editor::codes::edt("no-root-level"));
		EXPECT_EQ(a, 1);
	}

	TEST(InputBindingListTest, UnmatchedChordDoesNotInvoke) {
		InputBindingList list {};
		int a = 0;
		ASSERT_TRUE(list.addBinding(binding({ 1u, 2u }, &a)));

		EXPECT_FALSE(list.invoke(keys({ 1u })).has_value());
		EXPECT_EQ(a, 0);
	}

	TEST(InputBindingListTest, ConditionGatesInvocation) {
		InputBindingList list {};
		int a = 0;
		bool enabled = false;
		ASSERT_TRUE(list.addBinding(binding({ 1u }, &a, [&enabled] { return enabled; })));

		EXPECT_FALSE(list.invoke(keys({ 1u })).has_value());
		EXPECT_EQ(a, 0);

		enabled = true;
		EXPECT_TRUE(list.invoke(keys({ 1u })).has_value());
		EXPECT_EQ(a, 1);
	}

	TEST(InputServiceTest, AddRejectsDuplicateKey) {
		InputService service {};

		EXPECT_TRUE(service.addInputBindings(general_input_binding_list, make_uptr<InputBindingList>()));
		EXPECT_FALSE(service.addInputBindings(general_input_binding_list, make_uptr<InputBindingList>()));
	}

	TEST(InputServiceTest, LookupFindsRegisteredList) {
		InputService service {};
		auto list = make_uptr<InputBindingList>();
		const auto* const raw = list.get();
		ASSERT_TRUE(service.addInputBindings("Custom", std::move(list)));

		const auto found = service.getInputBindings(String { "Custom" });

		ASSERT_TRUE(found.has_value());
		EXPECT_EQ(&found.value(), raw);
		EXPECT_FALSE(service.getInputBindings(String { "Other" }).has_value());
	}
}
