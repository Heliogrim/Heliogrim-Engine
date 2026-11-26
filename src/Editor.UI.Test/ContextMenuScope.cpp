#include "pch.h"

/**/

#include <Engine.Common/__macro.hpp>

import Editor.UI;
import Editor.UI.Service;
import Engine.Common;
import Engine.Reflow;
import Engine.Reflow.Uikit;

using namespace hg;

namespace ReflowModule {
	namespace {
		[[nodiscard]] sptr<editor::ui::ContextMenuProvider> provider(
			string name_,
			sptr<engine::reflow::Widget> content_,
			ref<Vector<string>> log_
		) {
			auto result = make_sptr<editor::ui::ContextMenuProvider>();
			result->setContent(::hg::move(content_));
			result->onContextMenu(
				[name = ::hg::move(name_), log = &log_](const auto&) {
					// The log stands in for the entries; constructing real menu items needs the engine (button themes).
					log->emplace_back(name);
					return engine::reflow::EventResponse::eHandled;
				}
			);
			return result;
		}
	}

	/**
	 * window > outer > column > [ inner > leftLeaf, sibling > rightLeaf ]
	 */
	class ContextMenuScopeFixture :
		public testing::Test {
	public:
		Vector<string> log;
		editor::ui::service::ContextService service;
		sptr<engine::reflow::Window> window;
		sptr<engine::reflow::uikit::Paint> leftLeaf;
		sptr<engine::reflow::uikit::Paint> rightLeaf;
		sptr<editor::ui::ContextMenuProvider> outer;
		sptr<editor::ui::ContextMenuProvider> inner;
		sptr<editor::ui::ContextMenuProvider> sibling;

	public:
		void SetUp() override {
			leftLeaf = make_sptr<engine::reflow::uikit::Paint>();
			rightLeaf = make_sptr<engine::reflow::uikit::Paint>();
			inner = provider("inner", leftLeaf, log);
			sibling = provider("sibling", rightLeaf, log);

			auto column = make_sptr<engine::reflow::uikit::VerticalLayout>();
			column->addChild(inner);
			column->addChild(sibling);
			outer = provider("outer", column, log);

			window = make_sptr<engine::reflow::Window>();
			window->setContent(outer);
		}
	};

	TEST_F(ContextMenuScopeFixture, RightClickUsesNearestProviders) {
		ASSERT_EQ(window->getFocusTarget(), nullptr);

		const auto menu = service.buildContextMenu(*window, *leftLeaf);

		EXPECT_NE(menu, nullptr);
		EXPECT_TRUE(std::ranges::contains(log, string { "inner" }));
		EXPECT_TRUE(std::ranges::contains(log, string { "outer" }));
	}

	TEST_F(ContextMenuScopeFixture, NoGlobalStack) {
		::hg::discard = service.buildContextMenu(*window, *leftLeaf);
		EXPECT_FALSE(std::ranges::contains(log, string { "sibling" }));

		log.clear();
		::hg::discard = service.buildContextMenu(*window, *rightLeaf);
		EXPECT_FALSE(std::ranges::contains(log, string { "inner" }));
		EXPECT_TRUE(std::ranges::contains(log, string { "sibling" }));
	}

	TEST_F(ContextMenuScopeFixture, NestedOrderNearestFirst) {
		::hg::discard = service.buildContextMenu(*window, *leftLeaf);
		EXPECT_EQ(log, (Vector<string> { "inner", "outer" }));

		// The provider itself is the nearest scope of a right click on it.
		log.clear();
		::hg::discard = service.buildContextMenu(*window, *inner);
		EXPECT_EQ(log, (Vector<string> { "inner", "outer" }));
	}
}
