#include "pch.h"

/**/

#include "TreeFixture.hpp"

import Engine.Common;
import Engine.Reflow;
import Engine.Reflow.Uikit;

using namespace hg::engine::reflow;
using namespace hg;

namespace ReflowModule {
	namespace {
		struct ThemeContext {
			Cell<int> accent;
			Cell<int> size;

			[[nodiscard]] auto cells() noexcept {
				return std::tie(accent, size);
			}
		};

		struct DepthContext {
			static constexpr bool isStatic = true;
			int level = 0;
		};

		/* Runs a callback while a layout mark climbs through it, which is the only user code a notification reaches. */
		class HookProvider final :
			public ContextProvider<DepthContext> {
		public:
			std::function<void()> onBoundaryQuery;

			[[nodiscard]] bool isLayoutBoundary() const noexcept override {
				if (onBoundaryQuery) {
					onBoundaryQuery();
				}
				return false;
			}
		};

		void settle(ref<Widget> widget_) {
			std::ignore = widget_.clearLayoutPending();
			std::ignore = widget_.clearRenderPending();
			std::ignore = widget_.clearSubtreePending();
		}

		[[nodiscard]] sptr<ContextProvider<ThemeContext>> themeProvider(cref<sptr<Widget>> content_) {
			auto provider = make_sptr<ContextProvider<ThemeContext>>();
			provider->setContent(content_);
			return provider;
		}
	}

	TEST(Context, NearestProviderWins) {
		auto reader = make_sptr<ProbePaint>();
		auto inner = themeProvider(reader);
		auto outer = themeProvider(inner);

		const auto context = reader->context<ThemeContext>();
		ASSERT_TRUE(context.has_value());
		EXPECT_EQ(std::addressof(context.value()), std::addressof(inner->value()));

		// A provider resolves its own lookup from its parent.
		const auto fromInner = inner->context<ThemeContext>();
		ASSERT_TRUE(fromInner.has_value());
		EXPECT_EQ(std::addressof(fromInner.value()), std::addressof(outer->value()));

		EXPECT_FALSE(reader->context<DepthContext>().has_value());
	}

	TEST(Context, ReaderOfOtherFieldNotNotified) {
		auto reader = make_sptr<ProbePaint>();
		auto provider = themeProvider(reader);

		auto& theme = reader->context<ThemeContext>().value();
		EXPECT_EQ(theme.accent.read(*reader, ReflowPhase::ePaint), 0);
		settle(*reader);

		EXPECT_TRUE(theme.size.set(4));
		EXPECT_FALSE(reader->state().isRenderPending());
		EXPECT_TRUE(theme.size.readers().empty());

		EXPECT_TRUE(theme.accent.set(2));
		EXPECT_TRUE(reader->state().isRenderPending());
		EXPECT_EQ(theme.accent.peek(), 2);
	}

	TEST(Context, SetEqualValueNotifiesNobody) {
		auto reader = make_sptr<ProbePaint>();
		auto provider = themeProvider(reader);

		auto& theme = reader->context<ThemeContext>().value();
		std::ignore = theme.accent.read(*reader, ReflowPhase::ePaint);
		std::ignore = theme.accent.read(*reader, ReflowPhase::eLayout);
		settle(*reader);

		EXPECT_FALSE(theme.accent.set(0));
		EXPECT_FALSE(reader->state().isRenderPending());
		EXPECT_FALSE(reader->state().isLayoutPending());

		// Repeated reads merge into one record.
		ASSERT_EQ(theme.accent.readers().size(), 1uLL);
		EXPECT_TRUE(theme.accent.readers().front().phases & ReflowPhase::eLayout);
		EXPECT_TRUE(theme.accent.readers().front().phases & ReflowPhase::ePaint);
	}

	TEST(Context, ReparentReresolvesProvider) {
		auto reader = make_sptr<ProbePaint>();
		auto first = themeProvider(reader);
		auto second = themeProvider(nullptr);

		std::ignore = reader->context<ThemeContext>().value().accent.read(*reader, ReflowPhase::ePaint);
		second->setContent(reader);

		const auto context = reader->context<ThemeContext>();
		ASSERT_TRUE(context.has_value());
		EXPECT_EQ(std::addressof(context.value()), std::addressof(second->value()));

		settle(*reader);
		EXPECT_TRUE(first->value().accent.set(5));
		EXPECT_FALSE(reader->state().isRenderPending());
		EXPECT_TRUE(first->value().accent.readers().empty());

		std::ignore = second->value().accent.read(*reader, ReflowPhase::ePaint);
		EXPECT_TRUE(second->value().accent.set(6));
		EXPECT_TRUE(reader->state().isRenderPending());
	}

	TEST(Context, DeadReaderIsDropped) {
		auto reader = make_sptr<ProbePaint>();
		auto provider = themeProvider(reader);

		auto& accent = provider->value().accent;
		std::ignore = reader->context<ThemeContext>().value().accent.read(*reader, ReflowPhase::ePaint);
		ASSERT_EQ(accent.readers().size(), 1uLL);

		provider->setContent(make_sptr<ProbePaint>());
		reader.reset();

		EXPECT_TRUE(accent.set(3));
		EXPECT_TRUE(accent.readers().empty());
	}

	TEST(Context, StaticContextIsUntracked) {
		static_assert(ContextProvider<DepthContext>::isStatic);
		static_assert(not ContextProvider<ThemeContext>::isStatic);

		auto reader = make_sptr<ProbePaint>();
		auto provider = make_sptr<ContextProvider<DepthContext>>();
		provider->setContent(reader);

		EXPECT_EQ(reader->context<DepthContext>()->level, 0);
		settle(*reader);

		provider->value().level = 2;
		EXPECT_EQ(reader->context<DepthContext>()->level, 2);
		EXPECT_FALSE(reader->state().isRenderPending());
		EXPECT_FALSE(reader->state().isLayoutPending());
	}

	TEST(Context, OrphanClearsCache) {
		auto reader = make_sptr<ProbePaint>();
		auto provider = themeProvider(reader);
		ASSERT_TRUE(reader->context<ThemeContext>().has_value());

		provider->setContent(make_sptr<ProbePaint>());
		EXPECT_FALSE(reader->context<ThemeContext>().has_value());

		provider.reset();
		EXPECT_FALSE(reader->context<ThemeContext>().has_value());
	}

	TEST(Context, ReaderAddedDuringNotifyIsSafe) {
		auto reader = make_sptr<ProbePaint>();
		auto hook = make_sptr<HookProvider>();
		hook->setContent(reader);
		auto provider = themeProvider(hook);

		auto& accent = provider->value().accent;
		std::ignore = reader->context<ThemeContext>().value().accent.read(*reader, ReflowPhase::eLayout);
		hook->onBoundaryQuery = [&] {
			std::ignore = hook->context<ThemeContext>().value().accent.read(*hook, ReflowPhase::ePaint);
		};
		settle(*reader);
		settle(*hook);
		settle(*provider);

		EXPECT_TRUE(accent.set(1));
		EXPECT_TRUE(reader->state().isLayoutPending());
		ASSERT_EQ(accent.readers().size(), 2uLL);
		EXPECT_EQ(accent.readers()[1].widget.lock(), hook);

		settle(*reader);
		settle(*hook);
		settle(*provider);

		// The hook reads again during the second notification; the record stays single.
		EXPECT_TRUE(accent.set(2));
		EXPECT_EQ(accent.readers().size(), 2uLL);
		hook->onBoundaryQuery = nullptr;
	}

	TEST(Context, DestroyedProviderIsReresolved) {
		auto reader = make_sptr<ProbePaint>();
		auto provider = themeProvider(reader);
		ASSERT_TRUE(reader->context<ThemeContext>().has_value());

		// Destroyed without detaching its content; the cached provider expires with it.
		provider.reset();
		EXPECT_FALSE(reader->context<ThemeContext>().has_value());
	}

	TEST(Context, MovedProviderKeepsItsReaders) {
		auto reader = make_sptr<ProbePaint>();
		auto provider = themeProvider(reader);
		auto first = make_sptr<ContextProvider<DepthContext>>();
		auto second = make_sptr<ContextProvider<DepthContext>>();
		first->setContent(provider);

		auto& accent = provider->value().accent;
		std::ignore = reader->context<ThemeContext>().value().accent.read(*reader, ReflowPhase::ePaint);

		first->setContent(nullptr);
		second->setContent(provider);
		settle(*reader);

		EXPECT_TRUE(accent.set(7));
		EXPECT_TRUE(reader->state().isRenderPending());
		EXPECT_EQ(accent.readers().size(), 1uLL);
	}

	TEST(Context, ReaderRemovedDuringNotifyIsDropped) {
		auto reader = make_sptr<ProbePaint>();
		auto inner = make_sptr<HookProvider>();
		auto outer = make_sptr<HookProvider>();
		inner->setContent(reader);
		outer->setContent(inner);
		auto provider = themeProvider(outer);

		auto& accent = provider->value().accent;
		std::ignore = reader->context<ThemeContext>().value().accent.read(*reader, ReflowPhase::eLayout);
		std::ignore = inner->context<ThemeContext>().value().accent.read(*inner, ReflowPhase::ePaint);
		ASSERT_EQ(accent.readers().size(), 2uLL);

		// The reader's layout mark climbs through the outer hook, which detaches the later reader.
		bool detached = false;
		outer->onBoundaryQuery = [&] {
			if (not std::exchange(detached, true)) {
				outer->setContent(make_sptr<ProbePaint>());
			}
		};
		settle(*reader);
		settle(*inner);
		settle(*outer);
		settle(*provider);

		EXPECT_TRUE(accent.set(1));
		EXPECT_TRUE(detached);
		EXPECT_TRUE(reader->state().isLayoutPending());
		EXPECT_FALSE(inner->state().isRenderPending());
		ASSERT_EQ(accent.readers().size(), 1uLL);
		EXPECT_EQ(accent.readers().front().widget.lock(), reader);
		outer->onBoundaryQuery = nullptr;
	}
}
